#ifndef CODEGEN_H
#define CODEGEN_H
#include <stdio.h>
#include "tnode.h"

#define NUM_REGS 20
#define reg_index int

reg_index getReg(void);
void freeReg(void);

/* Generates code for an EXPRESSION subtree (NUM_T, ID_T, or an operator).
 * Returns the register holding the computed value. */
reg_index codeGen(struct tnode *t, FILE *target_file);

/* Generates code for a STATEMENT subtree (CONNECTOR_T, ASSIGN_T,
 * READ_T, WRITE_T). Produces no value — just emits instructions. */
void genStmt(struct tnode *t, FILE *target_file);

/* Writes the full XEXE file: header + SP init + statement code + exit call. */
void generate_code(struct tnode *root, const char *filename);

#endif