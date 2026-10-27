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

void appendClassMethodParams(node* paramList) {
    if(paramList == NULL) {
        return;
    }
    if(paramList->nodetype == NODE_PARAM) {
        ctAddMethodParam(paramList->varname, paramList->left->varname);
        return;
    }
    appendClassMethodParams(paramList->left);
    appendClassMethodParams(paramList->right);
}

void installClass(node* root) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_CDEF) {
        ctInstall(root->left->left->varname);
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
        appendClassMethodParams(root->right);
        return;
    }

    installClass(root->left);
    installClass(root->right);
}