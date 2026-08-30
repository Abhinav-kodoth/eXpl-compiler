#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "exptree.h"


#ifndef CODEGEN_H
#define CODEGEN_H


static int nums[26] = {0} ;

int getReg();
void freeReg();

int getAddress(struct tnode * root,FILE * op);  //Not needed since only helper function in codeGen.c
int codeGen(struct tnode * root, FILE * op);
int evaluate(struct tnode * root);
void inorder(struct tnode * root);


int getLabel() ;
void pushLabel(int breakLabel,int continueLabel);
void popLabel();
int getContinueLabel();
int getBreakLabel();

void auxFunctions(FILE * op, int codePrint, int reg1,int reg2);

#endif