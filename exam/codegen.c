#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"

static int regTop = -1;

int getReg(void) {
    regTop++;
    if (regTop >= NUM_REGS) {
        
        exit(1);
    }
    return regTop;
}

void freeReg(void) {
    if (regTop < 0) {
        
        exit(1);
    }
    regTop--;
}

int codeGen(struct tnode *t,FILE *target_file){
    if(!t)
    exit(1);

    if(t->op==NULL){
        int r = getReg();
        fprintf(target_file, "MOV R%d, %d\n", r, t->val);
        return r;
    }

    int p = codeGen(t->left, target_file);
    int q = codeGen(t->right, target_file);

     switch (t->op[0]) {
        case '+': fprintf(target_file, "ADD R%d, R%d\n", p, q); break;
        case '-': fprintf(target_file, "SUB R%d, R%d\n", p, q); break;
        case '*': fprintf(target_file, "MUL R%d, R%d\n", p, q); break;
        case '/': fprintf(target_file, "DIV R%d, R%d\n", p, q); break;
        default:
            fprintf(stderr, "Error: unknown operator '%s'\n", t->op);
            exit(1);
    }

    freeReg();   /* release the higher-numbered register, q */
    return p;





}


void generate_code(struct tnode *root,const char *filename){
    FILE *target_file=fopen(filename,"w");

    fprintf(target_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0, 2056, 0, 0, 0, 0, 0, 0);

    int  result = codeGen(root, target_file);

    fprintf(target_file, "MOV [4096], R%d\n", result);

    fprintf(target_file, "MOV SP, 4095\n");

    int sysno=getReg();

    fprintf(target_file, "MOV R%d, 5\n", sysno);     /* 5 = write system call number */
    fprintf(target_file, "PUSH R%d\n", sysno);

    int blank = getReg();          /* reused for arg1 and the two blank slots */
    fprintf(target_file, "MOV R%d, -2\n", blank);    /* argument 1: -2 (console) */
    fprintf(target_file, "PUSH R%d\n", blank);

    fprintf(target_file, "PUSH R%d\n", result);

    fprintf(target_file, "PUSH R%d\n", blank);       /* argument 3: blank */
    fprintf(target_file, "PUSH R%d\n", blank);        /* space for return value */

    fprintf(target_file, "INT 7\n");    
    
    fprintf(target_file, "POP R%d\n", result);        /* save return value (unused) */
    fprintf(target_file, "POP R%d\n", blank);
    fprintf(target_file, "POP R%d\n", blank);
    fprintf(target_file, "POP R%d\n", blank);
    fprintf(target_file, "POP R%d\n", sysno);

    freeReg(); /* blank */
    freeReg(); /* sysno */

    fclose(target_file);

}
