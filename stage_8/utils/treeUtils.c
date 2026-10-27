#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treeUtils.h"
#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../type_table/tt.h"
#include "../class_table/ct.h"
#include "../symbol_table/lst.h"
#include "../utils/utils.h"
#include "../utils/codegenUtils.h"

extern struct gst* gstRoot;
extern struct gst* lstRoot;

node* makeParamNode(node* type, node* var); // Forward declaration of makeParamNode to avoid implicit declaration warning in tree.c
node* makeLeafIdNode(char* varname); // Fw declaration

void enterParamsList(node* root, gst* gstEntry) {
    if(root == NULL) return;

    if(root->nodetype == NODE_PARAM) {
        int ptr_level = getDeclaredPtrLevel(root->right);
        classTable *cType = ctLookup(root->left->varname);
        appendParam(gstEntry, root->varname, root->type, cType, ptr_level);
        return;
    }
    enterParamsList(root->left, gstEntry);
    enterParamsList(root->right, gstEntry);
}

void assignType(node* root, typeTable* type, classTable* cType) {
    if(root == NULL) {
        return;
    }
    if(root->nodetype == NODE_ID) {
        int size = 1;
        char* varname = root->varname;

        if(cType) {
            size = 2;
        }

        gstInstall(varname, type, cType, size, 0);
        root->gstEntry = globalLookup(varname);
        root->type = type;
        return;
    }
    if(root->nodetype == NODE_ARRAY) {
        // Right node is guaranteed to be NUM due to grammar
        int size = root->right->val;
        char* varname = root->varname;

        gstInstall(varname, type, cType, size, 0);
        root->gstEntry = globalLookup(varname);
        root->left->gstEntry = root->gstEntry; 
        root->type = type;
        return; // You don't want to assign type to the ID child.
    }
    if(root->nodetype == NODE_PTR) {
        // Right node is the type being pointed to
        assignType(root->right, type, cType);
        root->varname = root->right->varname; 
        root->gstEntry = root->right->gstEntry;
        root->gstEntry->ptr_level++;
        root->type = type;
        return;
    }
    if(root->nodetype == NODE_FNDECL) {
        // Left node is the return type
        assignType(root->left, type, cType);
        root->varname = root->left->varname; 
        root->gstEntry = root->left->gstEntry;
        root->type = type;
        enterParamsList(root->right, root->gstEntry);
        return;
    }
    assignType(root->left, type, cType);
    assignType(root->right, type, cType);
}

void installType(node* root) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_TYPEDEF) {
        ttInstall(root->left->varname);
        installType(root->right);
        return;
    }

    if(root->nodetype == NODE_FIELDDECL) {
        ttAddField(root->left->varname, root->right->varname);
        return;
    }

    installType(root->left);
    installType(root->right);
}

void appendClassMethodParams(char* methodName, node* paramList) {
    if(paramList == NULL) {
        return;
    }
    if(paramList->nodetype == NODE_PARAM) {
        ctAddMethodParam(methodName, paramList->varname, paramList->left->varname);
        return;
    }
    appendClassMethodParams(methodName, paramList->left);
    appendClassMethodParams(methodName, paramList->right);
}

int checkParentChildMethodParams(char* methodName) {
    classTable* childClass = getLastClass();
    classTable* parentClass = childClass->parentClass;
    if(parentClass == NULL) {
        return 1; // No parent class, so no mismatch
    }

    cMethodList* childMethod = ctMethodLookup(childClass->name, methodName);
    cMethodList* parentMethod = ctMethodLookup(parentClass->name, methodName);
    if(parentMethod == NULL) {
        return 1; // No method in parent class, so no mismatch
    }

    paramStruct* childParam = childMethod->params;
    paramStruct* parentParam = parentMethod->params;
    while(childParam != NULL && parentParam != NULL) {
        if(childParam->type != parentParam->type) {
            return 0; // Mismatch found
        }
        childParam = childParam->next;
        parentParam = parentParam->next;
    }

    if(childParam != NULL || parentParam != NULL) {
        return 0; // Mismatch found (different number of parameters)
    }

    return 1; 
}

void installClass(node* root) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_EXTENDS) {
        attachParentClass(root->left->varname);
        copyClass(root->left->varname);
        installClass(root->right);
        return;
    }

    if(root->nodetype == NODE_CDEF) {
        // ctInstall(root->left->left->varname);
        installClass(root->left->right);
        installClass(root->right);
        return;
    }

    if(root->nodetype == NODE_CFIELD) {
        ctAddField(root->varname, root->left->varname);
        return;
    }

    if(root->nodetype == NODE_CMETHOD) {
        ctAddMethod(root->varname, root->left->varname);
        appendClassMethodParams(root->varname, root->right);
        if(getLastClass()->parentClass != NULL) {
            int paramsMatch = checkParentChildMethodParams(root->varname);
            if(!paramsMatch) {
                printf("Error: Parameter type mismatch in method '%s' between class '%s' and its parent class '%s'\n", root->varname, getLastClass()->name, getLastClass()->parentClass->name);
                exit(1);
            }
        }
        return;
    }

    if(root->nodetype == NODE_CFNDEF) {
        classTable* currClass = getLastClass();
        cMethodList* method = ctMethodLookup(currClass->name, root->varname);
        if(method == NULL) {
            printf("Error: Method '%s' not found in class '%s' for definition\n", root->varname, currClass->name);
            exit(1);
        }
        if(method->funcLabel < 0) {
            method->funcLabel = getFnLabel();
        }
        return;
    }

    installClass(root->left);
    installClass(root->right);
}