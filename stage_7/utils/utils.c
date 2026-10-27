#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../type_table/tt.h"
#include "../class_table/ct.h"

extern struct gst* gstRoot;
extern struct gst* lstRoot;

int getDeclaredPtrLevel(node* root) {
    int level = 0;
    node* current = root;
    while(current && current->nodetype == NODE_PTR) {
        level++;
        current = current->right;
    }
    return level;
}

typeHandle* createTypeHandleNode(typeTable* type, classTable* cType) {
    typeHandle* temp = (typeHandle*) malloc(sizeof(typeHandle));
    temp->type = type;
    temp->cType = cType;
    return temp;
}

typeHandle* getType(node* root) {
    if(root == NULL) {
        return NULL;
    }

    if(root->nodetype == NODE_FIELDFN) {
        typeHandle* leftType = getType(root->left);
        if(leftType == NULL) {
            printf("Error: Could not determine type of field function receiver\n");
            exit(1);
        }

        cMethodList *method = ctMethodLookup(leftType->cType->name, root->right->left->varname);
        if(method == NULL) {
            printf("Error: No method named '%s' in type '%s' for field function call\n", root->right->varname, leftType->type ? leftType->type->name : leftType->cType->name);
            exit(1);
        }
        free(leftType); // To avoid memory leak
        return createTypeHandleNode(method->type, NULL);
    }

    if(root->nodetype == NODE_FIELD) {
        typeHandle* left = getType(root->left);

        if(left->cType) {
            struct cFieldList* field = ctFieldLookup(left->cType->name, root->right->varname);
            if(field == NULL) {
                printf("Error: No field named '%s' in class '%s'\n", root->right->varname, left->cType->name);
                exit(1);
            }
            free(left);
            return createTypeHandleNode(field->type, field->cType);
        }

        if((left->type == NULL && left->cType == NULL) || left->type == ttLookup("int") || left->type == ttLookup("str") || left->type == ttLookup("bool") || left->type == ttLookup("null")) {
            printf("Error: Attempting to access field '%s' of non user-defined type '%s'\n", root->right->varname, (left && left->type) ? left->type->name : "unknown");
            exit(1);
        }

        fieldList* field = ttFieldLookup(left->type->name, root->right->varname);
        if(field == NULL) {
            printf("Error: No field named '%s' in user-defined type '%s'\n", root->right->varname, left->type->name);
            exit(1);
        }
        free(left);
        return createTypeHandleNode(field->type, NULL);
    }

    if(root->varname) {
        gst* gstEntry = globalLookup(root->varname);
        if(gstEntry != NULL) {
            return createTypeHandleNode(gstEntry->type, gstEntry->cType);
        }
    }
    return createTypeHandleNode(root->type, root->cType);
}

int checkTypeEquivalence(typeHandle *tHandle, typeTable *type, classTable *cType) {
    if(tHandle->type && tHandle->type != type) {
        return 0;
    }
    if(tHandle->cType && tHandle->cType != cType) {
        return 0;
    }
    return 1;
}

void assignTypesLocal(node* root, typeTable *type, classTable *cType, struct paramStruct* paramList) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_ID) {
        char* varname = root->varname;
        int ptr_level = 0;
        lstInstall(varname, type, cType, ptr_level);
        root->type = type;
        return;
    }

    if(root->nodetype == NODE_PTR) {
        char* varname = root->varname;
        int ptr_level = getDeclaredPtrLevel(root);
        lstInstall(varname, type, cType, ptr_level);
        root->type = type;
        return;
    }

    assignTypesLocal(root->left, type, cType, paramList);
    assignTypesLocal(root->right, type, cType, paramList);
}

void buildLST(node* root, struct paramStruct* paramList) {
    if(!root) return;

    if(root->nodetype == NODE_LDECL) {
        typeTable* type = ttLookup(root->left->varname);
        classTable* cType = ctLookup(root->left->varname);
        if(type == NULL && cType == NULL) {
            printf("Error: Undeclared type '%s' in local declaration\n", root->left->varname);
            exit(1);
        }
        assignTypesLocal(root->right, type, cType, paramList);
        return;
    }

    buildLST(root->left, paramList);
    buildLST(root->right, paramList);
}