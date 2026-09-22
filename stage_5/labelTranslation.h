#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct labelNode {
    char * labelName ;
    int address ;
    struct labelNode * next ;
};

struct labelAddressTable {
    struct labelNode * head ;
};

struct funcAddressNode {
    char * funcName ;
    int address ;
    struct funcAddressNode * next ;
};

struct funcAddressNode * addFunc(struct funcAddressNode * node, char * functionName, int addr);

int searchFuncAddr(struct funcAddressNode * node, char * functionName);

void printFunctionTable(struct funcAddressNode * node);

void addLabel(struct labelAddressTable *lat, char* labelName, int addr);

int searchLabel(struct labelAddressTable *lat,char* label);

void printLabelTable(struct labelAddressTable *lat);