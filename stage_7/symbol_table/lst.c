#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "../type_table/tt.h"
#include "lst.h"

gst* lstRoot = NULL;
int localBindingCounter = 1;

gst* createLstNode(char* name, typeTable* type, classTable* cType) {
    gst* newNode = (gst*) malloc(sizeof(gst));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->cType = cType;
    newNode->size = 1;
    newNode->binding = -1;
    newNode->paramList = NULL;
    newNode->fLabel = -1;
    newNode->next = NULL;
    return newNode;
}

gst* lstInstall(char* name, typeTable* type, classTable* cType, int ptr_level) {
    if(ttLookup(name) != NULL) {
        printf("Error: Cannot declare a variable with name '%s' as there exist a type with same name\n", name);
        exit(1);
    }
    
    gst* newNode = createLstNode(name, type, cType);
    newNode->ptr_level = ptr_level;

    if (lstRoot == NULL) {
        newNode->relativeBinding = localBindingCounter++;
        lstRoot = newNode;
        return newNode;
    }
    gst* temp = lstRoot;
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
    newNode->relativeBinding = localBindingCounter++;
    prev->next = newNode;
    return newNode;
}

gst* lstLookup(char* name) {
    if(name == NULL) {
        return NULL;
    }

    gst* temp = lstRoot;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

gst* globalLookup(char* name) {
    if(name == NULL) {
        return NULL;
    }

    gst* temp = lstLookup(name);
    if(temp) {
        return temp;
    }
    return gstLookup(name);
}

void freeLst() {
    gst* temp = lstRoot;
    while (temp != NULL) {
        gst* next = temp->next;
        free(temp->name);
        free(temp);
        temp = next;
    }
    localBindingCounter = 1; // Reset binding counter for next function
    lstRoot = NULL;
}

int bindParamsHelper(gst* lstRoot) {
    if(lstRoot == NULL) return -2;

    lstRoot->relativeBinding = bindParamsHelper(lstRoot->next) - 1;
    localBindingCounter--;
    return lstRoot->relativeBinding;
}

void bindParams() {
    gst* temp = lstRoot;
    bindParamsHelper(temp);
}