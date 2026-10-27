#ifndef GST_H
#define GST_H

#include "lst.h"

typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct classTable classTable; // Forward declaration of classTable from ct.h

typedef struct paramStruct {
    char* name;
    typeTable* type;
    classTable* cType;
    int ptr_level;
    struct paramStruct* next;
} paramStruct;

typedef struct gst {
    char* name;       // name of the variable
    typeTable* type;  // type of the variable
    classTable* cType;
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int relativeBinding;
    int ptr_level;    // 0 for normal variables, 1 for pointers, 2 for double pointers, etc.
    paramStruct* paramList; // List of parameters if the variable is a function
    int fLabel;       // Label for the function entry point in code generation
    struct gst *next;
} gst;

gst* gstLookup(char* name);
void gstInstall(char* name, typeTable* type, classTable* cType, int size, int ptr_level);
void appendParam(gst* gstEntry, char* name, typeTable* type, classTable* cType, int ptr_level);
void printGST(gst* head);
int getSP();
int reserveSpace(int size);

#endif