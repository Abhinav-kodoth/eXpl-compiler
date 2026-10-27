#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegenUtils.h"
#include "../tree/tree.h"
#include "utils.h"
#include "../symbol_table/gst.h"
#include "../type_table/tt.h"
#include "../class_table/ct.h"

typedef struct regStack {
    int reg;
    struct regStack* next;
    struct regStack* prev;
} regStack;

int regCount = 0;
int label = 0;
int fnlabel = 0;
labelStack* labelStackTop = NULL;

extern struct gst* gstRoot;
extern struct gst* lstRoot;
extern int codeGen(node* root, FILE* targetFile);

regStack* regStackTop = NULL;

regStack* createRegStackNode(int reg) {
    regStack* temp = (regStack*) malloc(sizeof(regStack));
    temp->reg = reg;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

void pushRegStack() {
    regStack* newNode = createRegStackNode(regCount);
    if(regStackTop != NULL) {
        regStackTop->next = newNode;
        newNode->prev = regStackTop;
    }
    regStackTop = newNode;
    regCount = 0;
}

void popRegStack() {
    if(regStackTop == NULL) {
        fprintf(stderr, "Error: Trying to pop from empty register stack\n");
        exit(1);
    }
    regStack* temp = regStackTop;
    regStackTop = regStackTop->prev;
    if(regStackTop != NULL) {
        regStackTop->next = NULL;
    }
    regCount = temp->reg;
    free(temp);
}

int getReg() {
    if(regCount < 20) {
        return regCount++;
    }
    fprintf(stderr, "Error: Out of registers\n");
    exit(1);
}

void freeReg() {
    if(regCount > 0) {
        regCount--;
    }
}

int getRegCount() {
    return regCount;
}

int getLabel() {
    return label++;
}

int getFnLabel() {
    return fnlabel++;
}

int binaryOpHandler(node* root, FILE* targetFile) {
    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);

    if(root->nodetype == NODE_EQ || root->nodetype == NODE_NEQ) {
        if(root->nodetype == NODE_EQ){
            fprintf(targetFile, "EQ R%d, R%d\n", leftReg, rightReg);
        }
        else if(root->nodetype == NODE_NEQ){
            fprintf(targetFile, "NE R%d, R%d\n", leftReg, rightReg);
        }
        freeReg();
        return leftReg;
    }

    switch(root->nodetype) {
        case NODE_PLUS:
            fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_MINUS:
            fprintf(targetFile, "SUB R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_MUL:
            fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_DIV:
            fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_MOD:
            fprintf(targetFile, "MOD R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_GT:
            fprintf(targetFile, "GT R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_GTE:
            fprintf(targetFile, "GE R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_LT:
            fprintf(targetFile, "LT R%d, R%d\n", leftReg, rightReg);
            break;
            case NODE_LTE:
            fprintf(targetFile, "LE R%d, R%d\n", leftReg, rightReg);
            break;
        default:
            break;
    }
    freeReg();
    return leftReg;
}

labelStack* createLabelStackNode(int cond, int end) {
    labelStack* temp = (labelStack*) malloc(sizeof(labelStack));
    temp->cond = cond;
    temp->end = end;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

void pushLabelStack(int cond, int end) {
    labelStack* newNode = createLabelStackNode(cond, end);
    if(labelStackTop != NULL) {
        newNode->next = labelStackTop;
        labelStackTop->prev = newNode;
    }
    labelStackTop = newNode;
}

labelStack* popLabelStack() {
    if(labelStackTop == NULL) {
        return NULL;
    }
    labelStack* temp = labelStackTop;
    labelStackTop = labelStackTop->next;
    if(labelStackTop != NULL) {
        labelStackTop->prev = NULL;
    }
    free(temp);
    return labelStackTop;
}
