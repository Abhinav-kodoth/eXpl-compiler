#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

#include "../tree/tree.h"

typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h

typedef struct labelStack {
    int cond;
    int end;
    struct labelStack* next;
    struct labelStack* prev;
} labelStack;

labelStack* createLabelStackNode(int cond, int end);
labelStack* pushLabelStack(labelStack* top, int cond, int end);
labelStack* popLabelStack(labelStack* top);

int getReg();
void freeReg();
int getRegCount();
int getLabel();
int getFnLabel();
void buildLST(node* root, gst** lst, struct paramStruct* paramList);
void pushRegStack();
void popRegStack();
int getDeclaredPtrLevel(node* root);
typeTable* getType(node* root);
void installType(node* root);

extern int regCount;
extern int label;

#endif