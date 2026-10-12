#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "tnode.h"

#define NUM_REGS 20

/*
 Both operations only need to track the highest register in use so far. */
int getReg(void);
void freeReg(void);

/* Recursively generate XSM assembly for the expression tree rooted at t,
 * writing instructions to target_file. Returns the register holding the
 * result of the subtree. */
int codeGen(struct tnode *t, FILE *target_file);

/* Writes the full XEXE file (header + expression code + store-to-4096 +
 * write syscall) for the tree rooted at root, to the file named filename. */
void generate_code(struct tnode *root, const char *filename);

#endif