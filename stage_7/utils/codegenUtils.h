#ifndef CODEGEN_UTILS_H
#define CODEGEN_UTILS_H
#include <stdio.h>

typedef struct node node; // Forward declaration of node from tree.h
typedef struct gst gst; // Forward declaration of gst from gst.h
typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct classTable classTable; // Forward declaration of classTable from ct.h

typedef struct labelStack {
    int cond;
    int end;
    struct labelStack* next;
    struct labelStack* prev;
} labelStack;

typedef struct typeHandle typeHandle;

void pushLabelStack(int cond, int end);
labelStack* popLabelStack();

int getReg();
void freeReg();
int getRegCount();
int getLabel();
int getFnLabel();
void pushRegStack();
void popRegStack();

#endif