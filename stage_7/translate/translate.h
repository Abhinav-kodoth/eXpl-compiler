#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct llnode {
    char* label;
    int address;
    struct llnode* next;
} llnode;

llnode* createLlNode(char* label, int address);
llnode* insert(char* label, int address, llnode* head);
int getAddress(char* label, llnode* head);
void translateLabels(FILE* inFile);

#endif