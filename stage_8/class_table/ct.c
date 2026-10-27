#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ct.h"
#include "../type_table/tt.h"
#include "../symbol_table/gst.h"
#include "../utils/codegenUtils.h"

classTable* ctRoot = NULL;

classTable* createClassTableNode(char *name) {
    classTable *temp = (classTable*) malloc(sizeof(struct classTable));
    temp->name = strdup(name);
    temp->classIndex = 0;
    temp->fieldCount = 0;
    temp->methodCount = 0;
    temp->vft = -1;
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
    temp->funcLabel = -2;
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
        if(ttLookup(name) != NULL || ctLookup(name) != NULL) {
            printf("Error: Can't create class '%s' - type with same namealready exists\n", name);
            exit(1);
        }
        if(ctLookup(name) != NULL) {
            printf("Error: Duplicate class '%s' in class table\n", name);
            exit(1);
        }
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
        if(strcmp(name, curr->name) == 0) {
            printf("Error: Duplicate field '%s' in class '%s'\n", name, root->name);
            exit(1);
        }
        curr = curr->next;
    }
    curr->next = temp;
    return;
}

void freeParamsList(paramStruct* params) {
    while(params != NULL) {
        paramStruct* temp = params;
        params = params->next;
        free(temp);
    }
}

void ctAddMethod(char* name, char* returnType) {
    cMethodList *temp = createClassMethodNode(name, returnType);

    if(ctRoot == NULL) {
        printf("Error: No classes in class table\n");
        exit(1);
    }

    classTable *root = getLastClass();

    cMethodList *curr = root->memberMethods;
    cMethodList *prev = NULL;
    while(curr != NULL) {
        if(strcmp(name, curr->name) == 0) {
            if(root->parentClass == NULL) {
                printf("Error: Duplicate method '%s' in class '%s'\n", name, root->name);
                exit(1);
            }

            classTable *parent = root->parentClass;
            cMethodList *parentMethod = ctMethodLookup(parent->name, name);
            if(parentMethod == NULL) {
                printf("Error: Method '%s' in class '%s' does not override any method in parent class '%s'\n", name, root->name, parent->name);
                exit(1);
            }
            if(parentMethod->type != temp->type) {
                printf("Error: Return type of method '%s' in class '%s' does not match return type of overridden method in parent class '%s'\n", name, root->name, parent->name);
                exit(1);
            }
            curr->funcLabel = -2; // Mark as method that needs to be defined
            freeParamsList(curr->params);
            curr->params = NULL;

            free(temp->name);
            free(temp);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    if(root->methodCount >= 8) {
        printf("Error: Class '%s' cannot have more than 8 methods\n", root->name);
        exit(1);
    }

    temp->funcIndex = root->methodCount++;
    if(root->memberMethods == NULL) {
        root->memberMethods = temp;
    }
    else {
        prev->next = temp;
    }
    return;
}

void ctAddMethodParam(char* methodName, char* paramName, char* typeName){
    paramStruct *temp = createClassMethodParamNode(paramName, typeName);

    if(ctRoot == NULL) {
        printf("Error: No classes in class table\n");
        exit(1);
    }

    classTable *root = getLastClass();

    if(root->memberMethods == NULL) {
        printf("Error: No methods in class '%s' to add parameter to\n", root->name);
        exit(1);
    }

    cMethodList *method = ctMethodLookup(root->name, methodName);
    if(method == NULL) {
        printf("Error: Method '%s' not found in class '%s' to add parameter to\n", methodName, root->name);
        exit(1);
    }

    if(method->params == NULL) {
        method->params = temp;
        return;
    }
    paramStruct *curr = method->params;
    while(curr->next != NULL) {
        if(strcmp(paramName, curr->name) == 0) {
            printf("Error: Duplicate parameter '%s' in method '%s' of class '%s'\n", paramName, method->name, root->name);
            exit(1);
        }
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

void attachParentClass(char* parentClassName) {
    classTable* childClass = getLastClass();
    classTable* parentClass = ctLookup(parentClassName);
    if(parentClass == NULL) {
        printf("Error: Parent class '%s' not found for class '%s'\n", parentClassName, childClass->name);
        exit(1);
    }
    if(childClass == parentClass) {
        printf("Error: Class '%s' cannot extend itself\n", childClass->name);
        exit(1);
    }
    childClass->parentClass = parentClass;
    return;
}

void copyClass(char* parentClassName) {
    classTable* parentClass = ctLookup(parentClassName);
    if(parentClass == NULL) {
        printf("Error: Parent class '%s' not found for copying\n", parentClassName);
        exit(1);
    }
    classTable* childClass = getLastClass();
    if(childClass == NULL) {
        printf("Error: No child class found to copy from parent class '%s'\n", parentClassName);
        exit(1);
    }

    cFieldList *parentField = parentClass->memberFields;
    while(parentField != NULL) {
        ctAddField(parentField->name, parentField->type ? parentField->type->name : parentField->cType->name);
        parentField = parentField->next;
    }

    cMethodList *parentMethod = parentClass->memberMethods;
    while(parentMethod != NULL) {
        ctAddMethod(parentMethod->name, parentMethod->type->name);
        paramStruct *parentParam = parentMethod->params;
        while(parentParam != NULL) {
            ctAddMethodParam(parentMethod->name, parentParam->name, parentParam->type->name);
            parentParam = parentParam->next;
        }
        // copy over the label
        cMethodList* childMethod = ctMethodLookup(childClass->name, parentMethod->name);
        childMethod->funcLabel = parentMethod->funcLabel;
        parentMethod = parentMethod->next;
    }
    return;
}

void ctPrint() {
    classTable *root = ctRoot;
    printf("Class Table:\n");
    while(root != NULL) {
        printf("Class Name: %s, Class Index: %d, VFT: %d\n", root->name, root->classIndex, root->vft);
        cFieldList *field = root->memberFields;
        while(field != NULL) {
            printf("\tField Name: %s, Field Type: %s, Field Index: %d\n", field->name, field->type ? field->type->name : field->cType->name, field->fieldIndex);
            field = field->next;
        }
        cMethodList *method = root->memberMethods;
        while(method != NULL) {
            printf("\tMethod Name: %s, Return Type: %s, Method Index: %d\n", method->name, method->type->name, method->funcIndex);
            paramStruct *param = method->params;
            while(param != NULL) {
                printf("\t\tParam Name: %s, Param Type: %s\n", param->name, param->type->name);
                param = param->next;
            }
            method = method->next;
        }
        root = root->next;
    }
}
