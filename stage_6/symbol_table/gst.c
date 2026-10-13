#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "../type_table/tt.h"
#include "gst.h"

int addr_counter = 4096;
gst* gstRoot = NULL;

gst* createGstNode(char* name, typeTable* type, int size) {
    gst* newNode = (gst*) malloc(sizeof(gst));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->size = size;
    newNode->binding = -1;
    newNode->relativeBinding = -1;
    newNode->ptr_level = 0;
    newNode->paramList = NULL;
    newNode->fLabel = -1;
    newNode->next = NULL;
    return newNode;
}

gst* gstInstall(gst* head, char* name, typeTable* type, int size, int ptr_level) {
    if(ttLookup(name) != NULL) {
        printf("Error: Cannot declare a variable with name '%s' as there exist a type with same name\n", name);
        exit(1);
    }
    
    gst* newNode = createGstNode(name, type, size);
    newNode->ptr_level = ptr_level;
    if (head == NULL) {
        newNode->binding = reserveSpace(size);
        return newNode;
    }
    gst* temp = head;
    gst* prev = NULL;
    while (temp != NULL) {
        prev = temp;
        if(strcmp(temp->name, name) == 0) {
            // Variable already exists
            free(newNode->name);
            free(newNode);
            printf("Error: Redeclaration of variable '%s'.\n", name);
            exit(1);
        }
        temp = temp->next;
    }
    newNode->binding = reserveSpace(size);
    prev->next = newNode;
    return head;
}

gst* gstLookup(gst* head, char* name) {
    gst* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void appendParam(gst* gstEntry, char* name, typeTable* type, int ptr_level) {
    paramStruct* newParam = (paramStruct*) malloc(sizeof(paramStruct));
    newParam->name = strdup(name);
    newParam->type = type;
    newParam->ptr_level = ptr_level;
    newParam->next = NULL;

    if(gstEntry->paramList == NULL) {
        gstEntry->paramList = newParam;
        return;
    }
    paramStruct* temp = gstEntry->paramList;
    while(temp->next != NULL) {
        temp = temp->next;
        if(strcmp(temp->name, name) == 0) {
            // Parameter already exists
            printf("Error: Redeclaration of parameter '%s' in function '%s'.\n", name, gstEntry->name);
            exit(1);
        }
    }
    temp->next = newParam;
}

void printGST(gst* head) {
    gst* temp = head;
    printf("Global Symbol Table:\n");
    printf("Name\tType\tSize\tBinding\tRBinding\tPtr_Level\n");
    while (temp != NULL) {
        char* typeStr = "";
        if(temp->type) typeStr = temp->type->name;
        printf("%s\t%s\t%d\t%d\t%d\t\t%d\n", temp->name, typeStr, temp->size, temp->binding, temp->relativeBinding, temp->ptr_level);
        if(temp->paramList) {
            printf("\tParameters:\n");
            printf("\tName\tType\tPtr_Level\n");
            paramStruct* paramTemp = temp->paramList;
            while (paramTemp != NULL) {
                const char* paramTypeStr = paramTemp->type->name;
                printf("\t%s\t%s\t%d\n", paramTemp->name, paramTypeStr, paramTemp->ptr_level);
                paramTemp = paramTemp->next;
            }
        }
        temp = temp->next;
    }
}

int getSP() {
    return addr_counter-1;
}

int reserveSpace(int size) {
    int temp = addr_counter;
    addr_counter += size;
    return temp;
}