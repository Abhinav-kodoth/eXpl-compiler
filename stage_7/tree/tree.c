#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "../utils/treeUtils.h"
#include "../utils/utils.h"
#include "../type_table/tt.h"
#include "../class_table/ct.h"

extern struct gst* gstRoot;
extern struct gst* lstRoot;

node* createTreeNode() {
    node* temp = (node*) malloc(sizeof(node));
    temp->varname = NULL;
    temp->type = NULL;
    temp->cType = NULL;
    temp->left = NULL;
    temp->right = NULL;
    temp->gstEntry = NULL;
    return temp;
}

node* makeLeafNumNode(int n) {
    node* temp = createTreeNode();
    temp->val = n;
    temp->type = ttLookup("int");
    temp->nodetype = NODE_NUM;
    temp->gstEntry = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeLeafStrNode(char* str) {
    node* temp = createTreeNode();
    temp->strval = strdup(str);
    temp->type = ttLookup("str");
    temp->nodetype = NODE_STR;
    temp->gstEntry = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeLeafIdNode(char* varname) {
    node* temp = createTreeNode();
    temp->varname = strdup(varname);
    temp->nodetype = NODE_ID;
    temp->gstEntry = globalLookup(varname);
    if(temp->gstEntry) {
        temp->type = temp->gstEntry->type;
        temp->cType = temp->gstEntry->cType;
    }
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeArrayNode(node* varname, node* sizeNode) {
    node* temp = createTreeNode();
    temp->varname = strdup(varname->varname);
    temp->nodetype = NODE_ARRAY;
    // GST entry would have been created when processing DECL node
    temp->gstEntry = globalLookup(varname->varname);
    if(temp->gstEntry) {
        temp->type = temp->gstEntry->type;
        temp->cType = temp->gstEntry->cType;
    }
    temp->left = varname;
    temp->right = sizeNode;
    return temp;
}

node* makeOpNode(char* op, node* left, node* right) {
    if(!left || !right) {
        printf("Error: NULL operand in makeOpNode\n");
        return NULL;
    }

    node* temp = (node*) malloc(sizeof(node));

    if(strcmp(op, "=") == 0) {
        temp->nodetype = NODE_ASSIGN;
        temp->type = right->type;
        temp->cType = right->cType;
    }
    else if(strcmp(op, "==") == 0) {
        temp->nodetype = NODE_EQ;
        temp->type = ttLookup("bool");
    }
    else if(strcmp(op, "!=") == 0) {
        temp->nodetype = NODE_NEQ;
        temp->type = ttLookup("bool");
    }
    else if(strcmp(op, "+") == 0) {
        temp->nodetype = NODE_PLUS;
        if(getType(temp)->type != ttLookup("int") || getType(temp)->type != ttLookup("int")) {
            temp->type = ttLookup("str");
        } else {
            temp->type = ttLookup("int");
        }
    }
    else if(strcmp(op, "-") == 0) { 
        temp->nodetype = NODE_MINUS;
        if(getType(temp)->type != ttLookup("int") || getType(temp)->type != ttLookup("int")) {
            temp->type = ttLookup("str");
        } else {
            temp->type = ttLookup("int");
        }
    }
    else if(strcmp(op, "*") == 0) {
        temp->nodetype = NODE_MUL;
        temp->type = ttLookup("int");
    }
    else if(strcmp(op, "%") == 0) {
        temp->nodetype = NODE_MOD;
        temp->type = ttLookup("int");
    }
    else if(strcmp(op, "/") == 0) {
        temp->nodetype = NODE_DIV;
        temp->type = ttLookup("int");
    }
    else if(strcmp(op, ">") == 0) {
        temp->nodetype = NODE_GT;
        temp->type = ttLookup("bool");
    }
    else if(strcmp(op, ">=") == 0) {
        temp->nodetype = NODE_GTE;
        temp->type = ttLookup("bool");
    }
    else if(strcmp(op, "<") == 0) {
        temp->nodetype = NODE_LT;
        temp->type = ttLookup("bool");
    }
    else if(strcmp(op, "<=") == 0) {
        temp->nodetype = NODE_LTE;
        temp->type = ttLookup("bool");
    }
    else {
        printf("Error: Unknown operator %s in makeOpNode\n", op);
        free(temp);
        return NULL;
    }

    temp->left = left;
    temp->right = right;
    return temp;
}

node* makeWriteNode(node* left) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_WRITE;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeReadNode(node *left) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_READ;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeConnectorNode(node* left, node* right) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CONNECTOR;
    temp->left = left;
    temp->right = right;
    return temp;
}

node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_IFELSE;
    temp->left = cond;
    temp->right = makeConnectorNode(ifStmt, elseStmt);
    return temp;
}

node* makeIfNode(node* cond, node* ifStmt) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_IF;
    temp->left = cond;
    temp->right = ifStmt;
    return temp;
}

node* makeWhileNode(node* cond, node* body) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_WHILE;
    temp->left = cond;
    temp->right = body;
    return temp;
}

node* makeBreakNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_BREAK;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeContinueNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CONTINUE;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeBrkpNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_BRKP;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeDoWhileNode(node* body, node* cond) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_DOWHILE;
    temp->left = cond;
    temp->right = body;
    return temp;
}

node* makeDeclNode(node* type, node* varlist) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_DECL;
    temp->left = type;
    temp->right = varlist;
    assignType(varlist, ttLookup(type->varname), ctLookup(type->varname));
    return temp;
}

node* makeTypeNode(char* type) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_TYPE;
    temp->type = ttLookup(type);
    temp->cType = ctLookup(type);
    return temp;
}

node* makePtrNode(node* ptrTo) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_PTR;
    temp->varname = strdup(ptrTo->varname);
    temp->type = ptrTo->type;
    temp->cType = ptrTo->cType;
    if(ptrTo->gstEntry) {
        temp->gstEntry = ptrTo->gstEntry;
        // Level of PTR would have already been updated in decl section
        // Level of all the child PTR nodes are irrelevant here.
    }
    temp->left = NULL;
    temp->right = ptrTo;
    return temp;
}

node* makeAddressNode(node* var) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_ADDR_OF;
    temp->varname = strdup(var->varname);
    temp->type = var->type;
    temp->cType = var->cType;
    if(var->gstEntry) {
        temp->gstEntry = var->gstEntry;
        temp->varname = var->varname;
        temp->val = temp->gstEntry->binding;
    }
    temp->left = NULL;
    temp->right = var;
    return temp;
}

node* makeNullNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_NULL;
    temp->type = ttLookup("null");
    return temp;
}

node* makeFnDeclNode(node* name, node* paramList) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FNDECL;
    temp->varname = strdup(name->varname);
    temp->left = name;
    temp->right = paramList;
    return temp;
}

node* makeFnDefNode(node* type, node* name, node* paramList, node* lDeclBlock, node* body) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FNDEF;
    temp->varname = strdup(name->varname);
    temp->gstEntry = gstLookup(name->varname);
    temp->type = ttLookup(type->varname);
    temp->cType = ctLookup(type->varname);
    temp->left = paramList;
    temp->right = makeConnectorNode(lDeclBlock, body);
    free(type);
    free(name);
    return temp;
}

node* makeParamNode(node* type, node* var) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_PARAM;
    temp->varname = strdup(var->varname);
    temp->type = ttLookup(type->varname);
    temp->cType = ctLookup(type->varname);
    temp->left = type;
    temp->right = var;
    return temp;
}

node* makeReturnNode(node* retVal) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_RETURN;
    temp->type = retVal->type;
    temp->cType = retVal->cType;
    temp->left = retVal;
    temp->right = NULL;
    return temp;
}

node* makeFnCallNode(node* fnName, node* argList) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FNCALL;
    temp->varname = strdup(fnName->varname);
    temp->gstEntry = globalLookup(fnName->varname);
    if(temp->gstEntry) {
        temp->type = temp->gstEntry->type;
        temp->cType = temp->gstEntry->cType;
    }
    temp->left = fnName;
    temp->right = argList;
    return temp;
}

node* makeMainNode(node* lDeclBlock, node* body) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_MAIN;
    temp->left = lDeclBlock;
    temp->right = body;
    return temp;
}

node* makeLDeclNode(node* type, node* varlist) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_LDECL;
    temp->left = type;
    temp->right = varlist;
    return temp;
}

node* makeArgNode(node* expr) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_ARG;
    temp->type = expr->type;
    temp->cType = expr->cType;
    temp->left = expr;
    temp->right = NULL;
    return temp;
}

node* makeTypeDefNode(node* typeName, node* fieldList) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_TYPEDEF;
    temp->varname = strdup(typeName->varname);
    temp->left = typeName;
    temp->right = fieldList;
    installType(temp);
    return temp;
}

node* makeFieldDeclNode(node* typeName, node* fieldName) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FIELDDECL;
    temp->left = typeName;
    temp->right = fieldName;
    return temp;
}

node* makeInitializeNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_INITIALIZE;
    temp->type = ttLookup("int");
    return temp;
}

node* makeFreeNode(node* arg) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FREE;
    temp->left = arg;
    temp->type = ttLookup("int");
    return temp;
}

node* makeAllocNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_ALLOC;
    temp->type = ttLookup("int");
    return temp;
}

node* makeFieldNode(node* var, node* field) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FIELD;
    temp->varname = strdup(field->varname);
    temp->left = var;   // Has to be evaluated recursively
    temp->right = field;
    return temp;
}

node* makeFieldFnNode(node* var, node* field, node* args) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_FIELDFN;
    temp->left = var;
    temp->right = makeConnectorNode(field, args);
    return temp;
}

node* makeClassFnDefNode(node* root) {
    if(root == NULL) return NULL;

    if(root->nodetype == NODE_FNDEF) {
        root->nodetype = NODE_CFNDEF;
        return root;
    }

    makeClassFnDefNode(root->left);
    makeClassFnDefNode(root->right);
    return root;
}

node* makeClassMethodNode(node* type, node* name, node* params) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CMETHOD;
    temp->varname = strdup(name->varname);
    temp->left = type;
    temp->right = params;
    return temp;
}

node* makeClassFieldNode(node* type, node* name) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CFIELD;
    temp->varname = strdup(name->varname);
    temp->left = type;
    temp->right = name;
    return temp;
}

node* makeClassDefNode(node* name, node* fieldList, node* methodList, node* methodDefList) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CDEF;
    temp->varname = strdup(name->varname);
    temp->left = makeConnectorNode(name, fieldList);
    temp->right = makeConnectorNode(methodList, methodDefList);
    installClass(temp);
    return temp;
}

node* makeNewNode(node* className) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_NEW;
    temp->varname = strdup(className->varname);
    temp->cType = ctLookup(className->varname);
    temp->left = className;
    return temp;
}