#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt.h"

typeTable *ttHead = NULL;

typeTable* createTypeTableNode(char *name) {
    typeTable *temp = (typeTable*)malloc(sizeof(typeTable));
    temp->name = strdup(name);
    temp->size = 0;
    temp->fields = NULL;
    temp->next = NULL;
    return temp;
}

fieldList* createFieldListNode(char* name, char *typeName) {
    fieldList *temp = (fieldList*)malloc(sizeof(fieldList));
    temp->name = strdup(name);
    temp->fieldIndex = 0;
    temp->type = ttLookup(typeName);
    temp->next = NULL;
    return temp;
}

void ttInitialize() {
    ttInstall("int");
    ttInstall("str");
    ttInstall("null");
    ttInstall("bool");
}

typeTable* ttInstall(char *name) {
    typeTable *temp = createTypeTableNode(name);
    if(ttHead == NULL) {
        ttHead = temp;
        return temp;
    }
    typeTable *curr = ttHead;
    while(curr->next != NULL) {
        curr = curr->next;
        if(strcmp(curr->name, name) == 0) {
            printf("Error: Type '%s' already exists in type table\n", name);
            exit(1);
        }
    }
    curr->next = temp;
    return temp;
}

typeTable* ttLookup(char *name) {
    typeTable *curr = ttHead;
    while(curr != NULL) {
        if(strcmp(curr->name, name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void ttAddField(char* typeName, char* name) {
    if(ttHead == NULL) {
        printf("Error: No types in type table\n");
        exit(1);
    }
    typeTable *ttEntry = ttHead;
    while(ttEntry->next != NULL) {
        ttEntry = ttEntry->next;
    }

    fieldList *newField = createFieldListNode(name, typeName);
    if(newField->type == NULL) {
        printf("Error: Type '%s' not found in type table\n", typeName);
        exit(1);
    }
    newField->fieldIndex = ttEntry->size;
    ttEntry->size++;

    if(ttEntry->size > 8) {
        printf("Error: Type '%s' cannot have more than 8 fields\n", ttEntry->name);
        exit(1);
    }

    if(ttEntry->fields == NULL) {
        ttEntry->fields = newField;
        return;
    }
    fieldList *curr = ttEntry->fields;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newField;
}

fieldList* ttFieldLookup(char* type, char* fieldName) {
    typeTable *ttEntry = ttLookup(type);
    if(ttEntry == NULL) {
        printf("Error: Type '%s' not found in type table\n", type);
        exit(1);
    }
    fieldList *curr = ttEntry->fields;
    while(curr != NULL) {
        if(strcmp(curr->name, fieldName) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void ttPrint() {
    typeTable *curr = ttHead;
    printf("Type Table:\n");
    while(curr != NULL) {
        printf("Type: %s, Size: %d\n", curr->name, curr->size);
        fieldList *fieldCurr = curr->fields;
        while(fieldCurr != NULL) {
            printf("  Field: %s, Type: %s, Index: %d\n", fieldCurr->name, fieldCurr->type->name, fieldCurr->fieldIndex);
            fieldCurr = fieldCurr->next;
        }
        curr = curr->next;
    }
    printf("\n");
}
