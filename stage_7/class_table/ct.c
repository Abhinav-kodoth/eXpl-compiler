#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ct.h"
#include "../type_table/tt.h"
#include "../symbol_table/gst.h"

classTable* ctRoot = NULL;

classTable* createClassTableNode(char *name) {
    classTable *temp = (classTable*) malloc(sizeof(struct classTable));
    temp->name = strdup(name);
    temp->classIndex = 0;
    temp->fieldCount = 0;
    temp->methodCount = 0;
    temp->memberFields = NULL;
    temp->memberMethods = NULL;
    temp->parentClass = NULL;
    temp->next = NULL;
    return temp;
}

cFieldList *createClassFieldNode(char *name, char *classOrTypeName) {
    cFieldList *temp = (cFieldList*) malloc(sizeof(struct cFieldList));
    temp->name = strdup(name);
    temp->fieldIndex = 0;
    temp->next = NULL;

    classTable *cType = ctLookup(classOrTypeName);
    typeTable *type = ttLookup(classOrTypeName);

    if(cType != NULL) {
        temp->cType = cType;
        temp->type = NULL;
    }
    else if(type != NULL) {
        temp->cType = NULL;
        temp->type = type;
    }
    else{
        printf("Error: Unknown class/type '%s' used for field '%s' in class definition\n", classOrTypeName, name);
        exit(1);
    }

    return temp;
}

cMethodList *createClassMethodNode(char* name, char* returnType) {
    cMethodList *temp = (cMethodList*) malloc(sizeof(struct cMethodList));
    temp->name = strdup(name);
    temp->funcIndex = 0;
    temp->funcLabel = 0;
    temp->params = NULL;
    temp->next = NULL;

    temp->type = ttLookup(returnType);
    if(temp->type == NULL) {
        printf("Error: Unknown return type '%s' for method '%s' in class definition\n", returnType, name);
        exit(1);
    }

    return temp;
}

paramStruct *createClassMethodParamNode(char* name, char* typeName) {
    paramStruct *temp = (paramStruct*) malloc(sizeof(struct paramStruct));
    temp->name = strdup(name);
    temp->next = NULL;

    temp->type = ttLookup(typeName);
    if(temp->type == NULL) {
        printf("Error: Unknown type '%s' for parameter '%s' in class method definition\n", typeName, name);
        exit(1);
    }

    return temp;
}

classTable *ctLookup(char *name) {
    if(ctRoot == NULL) return NULL;

    classTable *root = ctRoot;
    while(root != NULL) {
        if(strcmp(name, root->name) == 0) 
            return root;
        root = root->next;
    }

    return NULL;
}

void ctInstall(char *name) {
    classTable *temp = createClassTableNode(name);
    
    if(ctRoot == NULL){
        ctRoot = temp;
        return;
    }
    
    classTable *root = ctRoot;
    temp->classIndex++;
    while(root->next != NULL) {
        root = root->next;
        temp->classIndex++;
    }
    root->next = temp;
    return;
}

void ctAddField(char* name, char* classOrTypeName) {
    cFieldList *temp = createClassFieldNode(name, classOrTypeName);

    if(ctRoot == NULL) {
        printf("Error: No classes in class table\n");
        exit(1);
    }

    classTable *root = ctRoot;
    while(root->next != NULL) {
        root = root->next;
    }

    temp->fieldIndex = root->fieldCount++;
    if(root->fieldCount > 8) {
        printf("Error: Class '%s' cannot have more than 8 fields\n", root->name);
        exit(1);
    }

    if(root->memberFields == NULL) {
        root->memberFields = temp;
        return;
    }
    cFieldList *curr = root->memberFields;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = temp;
    return;
}

void ctAddMethod(char* name, char* returnType) {
    cMethodList *temp = createClassMethodNode(name, returnType);

    if(ctRoot == NULL) {
        printf("Error: No classes in class table\n");
        exit(1);
    }

    classTable *root = ctRoot;
    while(root->next != NULL) {
        root = root->next;
    }

    temp->funcIndex = root->methodCount++;
    if(root->methodCount > 8) {
        printf("Error: Class '%s' cannot have more than 8 methods\n", root->name);
        exit(1);
    }

    if(root->memberMethods == NULL) {
        root->memberMethods = temp;
        return;
    }
    cMethodList *curr = root->memberMethods;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = temp;
    return;
}    

void ctAddMethodParam(char* name, char* typeName){
    paramStruct *temp = createClassMethodParamNode(name, typeName);

    if(ctRoot == NULL) {
        printf("Error: No classes in class table\n");
        exit(1);
    }

    classTable *root = ctRoot;
    while(root->next != NULL) {
        root = root->next;
    }

    if(root->memberMethods == NULL) {
        printf("Error: No methods in class '%s' to add parameter to\n", root->name);
        exit(1);
    }

    cMethodList *method = root->memberMethods;
    while(method->next != NULL) {
        method = method->next;
    }

    if(method->params == NULL) {
        method->params = temp;
        return;
    }
    paramStruct *curr = method->params;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = temp;
}

cFieldList* ctFieldLookup(char* className, char* fieldName) {
    classTable *cType = ctLookup(className);
    if(cType == NULL) return NULL;

    cFieldList *field = cType->memberFields;
    while(field != NULL) {
        if(strcmp(fieldName, field->name) == 0) {
            return field;
        }
        field = field->next;
    }

    return NULL;
}

cMethodList* ctMethodLookup(char* className, char* methodName) {
    classTable *cType = ctLookup(className);
    if(cType == NULL) return NULL;

    cMethodList *method = cType->memberMethods;
    while(method != NULL) {
        if(strcmp(methodName, method->name) == 0) {
            return method;
        }
        method = method->next;
    }

    return NULL;
}

classTable* getLastClass() {
    if(ctRoot == NULL) return NULL;

    classTable *root = ctRoot;
    while(root->next != NULL) {
        root = root->next;
    }
    return root;
}