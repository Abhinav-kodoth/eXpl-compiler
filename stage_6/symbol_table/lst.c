#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "../type_table/tt.h"
#include "lst.h"

gst* lstRoot = NULL;
int localBindingCounter = 1;

gst* createLstNode(char* name, typeTable* type) {
    gst* newNode = (gst*) malloc(sizeof(gst));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->size = 1;
    newNode->binding = -1;
    newNode->paramList = NULL;
    newNode->fLabel = -1;
    newNode->next = NULL;
    return newNode;
}

gst* lstInstall(gst* head, char* name, typeTable* type, int ptr_level) {
    if(ttLookup(name) != NULL) {
        printf("Error: Cannot declare a variable with name '%s' as there exist a type with same name\n", name);
        exit(1);
    }
    
    gst* newNode = createLstNode(name, type);
    newNode->ptr_level = ptr_level;

    if (head == NULL) {
        newNode->relativeBinding = localBindingCounter++;
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
    newNode->relativeBinding = localBindingCounter++;
    prev->next = newNode;
    return head;
}

gst* lstLookup(gst* head, char* name) {
    gst* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

gst* globalLookup(gst* gstHead, gst* lstHead, char* name) {
    gst* temp = lstLookup(lstHead, name);
    if(temp) {
        return temp;
    }
    return gstLookup(gstHead, name);
}

void freeLst(gst* head) {
    gst* temp = head;
    while (temp != NULL) {
        gst* next = temp->next;
        free(temp->name);
        free(temp);
        temp = next;
    }
    localBindingCounter = 1; // Reset binding counter for next function
}

int bindParams(gst* head) {
    if(head == NULL) return -2;

    head->relativeBinding = bindParams(head->next) - 1;
    localBindingCounter--;
    return head->relativeBinding;
}