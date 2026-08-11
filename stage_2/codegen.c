#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"

static int regTop = -1;

reg_index getReg(void) {
    regTop++;
    if (regTop >= NUM_REGS) {
        fprintf(stderr, "Error: Out of registers\n");
        exit(1);
    }
    return regTop;
}

void freeReg(void) {
    if (regTop < 0) {
        fprintf(stderr, "Error: freeReg() called with no register allocated\n");
        exit(1);
    }
    regTop--;
}

/* Maps a variable name ('a'-'z') to its statically allocated address. */
static int varAddress(char *varname) {
    return 4096 + (varname[0] - 'a');
}

/* ---------- EXPRESSION code generation ---------- */

reg_index codeGen(struct tnode *t, FILE *target_file) {
    if (t == NULL) {
        fprintf(stderr, "Error: codeGen called on NULL node\n");
        exit(1);
    }

    reg_index r;
    switch (t->nodetype) {
        case NUM_T:
            r = getReg();
            fprintf(target_file, "MOV R%d, %d\n", r, t->val);
            return r;

        case ID_T: {
            /* XSM only supports register-indirect addressing ([Rn]),
             * not a literal address directly inside brackets.
             * So: load the address into a register first, then
             * dereference that same register to get the value. */
            reg_index addrReg = getReg();
            fprintf(target_file, "MOV R%d, %d\n", addrReg, varAddress(t->varname));
            fprintf(target_file, "MOV R%d, [R%d]\n", addrReg, addrReg);
            return addrReg;
        }

        case PLUS_T:
        case MINUS_T:
        case MUL_T:
        case DIV_T: {
            reg_index p = codeGen(t->left, target_file);
            reg_index q = codeGen(t->right, target_file);
            const char *instr;
            switch (t->nodetype) {
                case PLUS_T:  instr = "ADD"; break;
                case MINUS_T: instr = "SUB"; break;
                case MUL_T:   instr = "MUL"; break;
                case DIV_T:   instr = "DIV"; break;
                default:      instr = "???"; break;
            }
            fprintf(target_file, "%s R%d, R%d\n", instr, p, q);
            freeReg();   /* release q, the higher register */
            return p;
        }

        default:
            fprintf(stderr, "Error: codeGen called on a non-expression node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

/* ---------- Raw system-call helpers (INT-based, as in Experiment II) ---------- */

static void genRead(FILE *f, int addr) {
    reg_index sysno = getReg();
    fprintf(f, "MOV R%d, 6\n", sysno);
    fprintf(f, "PUSH R%d\n", sysno);

    reg_index blank = getReg();
    fprintf(f, "MOV R%d, -2\n", blank);
    fprintf(f, "PUSH R%d\n", blank);

    reg_index addrReg = getReg();
    fprintf(f, "MOV R%d, %d\n", addrReg, addr);
    fprintf(f, "PUSH R%d\n", addrReg);

    fprintf(f, "PUSH R%d\n", blank);   /* arg 3, unused */
    fprintf(f, "PUSH R%d\n", blank);   /* return value slot */
    fprintf(f, "INT 6\n");

    fprintf(f, "POP R%d\n", addrReg);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", sysno);

    freeReg();   /* addrReg */
    freeReg();   /* blank */
    freeReg();   /* sysno */
}

static void genWrite(FILE *f, reg_index valueReg) {
    reg_index sysno = getReg();
    fprintf(f, "MOV R%d, 5\n", sysno);
    fprintf(f, "PUSH R%d\n", sysno);

    reg_index blank = getReg();
    fprintf(f, "MOV R%d, -2\n", blank);
    fprintf(f, "PUSH R%d\n", blank);

    fprintf(f, "PUSH R%d\n", valueReg);
    fprintf(f, "PUSH R%d\n", blank);
    fprintf(f, "PUSH R%d\n", blank);
    fprintf(f, "INT 7\n");

    fprintf(f, "POP R%d\n", valueReg);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", blank);
    fprintf(f, "POP R%d\n", sysno);

    freeReg();   /* blank */
    freeReg();   /* sysno */
}

static void genExit(FILE *f) {
    reg_index sysno = getReg();
    fprintf(f, "MOV R%d, 10\n", sysno);
    fprintf(f, "PUSH R%d\n", sysno);
    reg_index blank = getReg();
    fprintf(f, "PUSH R%d\n", blank);
    fprintf(f, "PUSH R%d\n", blank);
    fprintf(f, "PUSH R%d\n", blank);
    fprintf(f, "INT 7\n");
}

/* ---------- STATEMENT code generation ---------- */

void genStmt(struct tnode *t, FILE *target_file) {
    if (t == NULL) return;

    switch (t->nodetype) {
        case CONNECTOR_T:
            genStmt(t->left, target_file);
            genStmt(t->right, target_file);
            break;

        case ASSIGN_T: {
            reg_index r = codeGen(t->right, target_file);
            reg_index addrReg = getReg();
            fprintf(target_file, "MOV R%d, %d\n", addrReg, varAddress(t->left->varname));
            fprintf(target_file, "MOV [R%d], R%d\n", addrReg, r);
            freeReg();   /* addrReg */
            freeReg();   /* r */
            break;
        }

        case READ_T: {
            int addr = varAddress(t->left->varname);
            genRead(target_file, addr);
            break;
        }

        case WRITE_T: {
            reg_index r = codeGen(t->left, target_file);
            genWrite(target_file, r);
            freeReg();
            break;
        }

        default:
            fprintf(stderr, "Error: genStmt called on a non-statement node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

/* ---------- Top-level driver ---------- */

void generate_code(struct tnode *root, const char *filename) {
    FILE *target_file = fopen(filename, "w");
    if (target_file == NULL) {
        fprintf(stderr, "Error: could not open '%s' for writing\n", filename);
        exit(1);
    }

    fprintf(target_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0, 2056, 0, 0, 0, 0, 0, 0);
    fprintf(target_file, "MOV SP, 4095\n");

    genStmt(root, target_file);
    genExit(target_file);

    fclose(target_file);
}