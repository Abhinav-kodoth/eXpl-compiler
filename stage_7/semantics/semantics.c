#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../utils/utils.h"
#include "../type_table/tt.h"
#include "../symbol_table/lst.h"
#include "../class_table/ct.h"

extern struct gst* gstRoot;
extern struct gst* lstRoot;

classTable *currClass = NULL;
int fieldLevel = 0;

void enforcePrivateAttributes(node* root) {
    if(root == NULL) {
        return;
    }
    
    if(root->nodetype == NODE_FIELD) {
        enforcePrivateAttributes(root->left);
        typeHandle* type = getType(root->left);
        if(type->cType && type->cType != currClass) {
            printf("\n%p %p\n", type->cType, currClass);
            printf("Semantics Error: Attempting to access private field '%s' of class '%s' from outside the class\n", root->right->varname, type->cType->name);
            exit(1);
        }
    }
}

void matchParamsList(struct paramStruct** paramList, node* argList) {
    if(paramList == NULL || *paramList == NULL) {
        if(argList == NULL) {
            return;
        }
        if(argList->nodetype == NODE_CONNECTOR) {
            matchParamsList(paramList, argList->left);
            matchParamsList(paramList, argList->right);
            return;
        }
        printf("Semantics Error: Too many arguments in function definition/call\n");
        exit(1);
    }

    if(argList == NULL) {
        return;
    }
    
    if(argList->nodetype == NODE_CONNECTOR) {
        matchParamsList(paramList, argList->left);
        matchParamsList(paramList, argList->right);
        return;
    }

    if(argList->nodetype == NODE_PARAM) {
        if(strcmp(argList->varname, "self") == 0) {
            // skip it, it's an implicit parameter
            return;
        }
        if(!checkTypeEquivalence(getType(argList), (*paramList)->type, (*paramList)->cType)) {
            printf("Semantics Error: Fn def - Parameter type mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
        if(strcmp((*paramList)->name, argList->right->varname) != 0) {
            printf("Semantics Error: Fn def - Parameter name mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
    }

    if(argList->nodetype == NODE_ARG) {
        if(!checkTypeEquivalence(getType(argList->left), (*paramList)->type, (*paramList)->cType)) {
            printf("Semantics Error: Fn call - Parameter type mismatch for parameter '%s' '%s'\n", (*paramList)->name, (*paramList)->type->name);
            exit(1);
        }
        if((*paramList)->ptr_level != getDerefLevel(argList->left)) {
            printf("Semantics Error: Fn call - Dereference level mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
    }

    *paramList = (*paramList)->next;
    return;
}

int getDerefLevel(node* root) {
    if(root == NULL) {
        return 0;
    }
    if(root->nodetype == NODE_PTR) {
        return getDerefLevel(root->right) - 1;
    }
    if(root->nodetype == NODE_ADDR_OF) {
        return getDerefLevel(root->right) + 1;
    }
    if(root->nodetype == NODE_ID || root->nodetype == NODE_ARRAY) {
        gst*  entry = globalLookup(root->varname);
        return entry ? entry->ptr_level : 0;
    }
    int left = getDerefLevel(root->left);
    int right = getDerefLevel(root->right);
    return left > right ? left : right;
}

void semantics(node* root) {
    if(root == NULL) {
        return;
    }

    switch(root->nodetype) {
        case NODE_READ:
        case NODE_WRITE: {
            semantics(root->left);

            if(getDerefLevel(root->left) != 0) {
                printf("Semantics Error: Cannot read/write pointer variable '%s' without dereferencing\n", root->left->varname);
                exit(1);
            }
            
            return;
        }
        case NODE_ASSIGN: {
            // Left can be ID, PTR, ARRAY or ADDR_OF
            // Right can be any expr

            node* left = root->left; 
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(root->right->nodetype == NODE_NEW) {                
                typeHandle *leftType = getType(root->left);
                if(leftType->cType != root->right->cType) {
                    printf("Semantics Error: Type mismatch in assignment instanciation of object '%s' with class '%s'\n", root->left->varname, root->right->cType->name);
                    exit(1);
                }
            }

            if(left->nodetype == NODE_ID && strcmp(left->varname, "self") == 0) {
                printf("Semantics Error: Cannot assign to 'self'\n");
                exit(1);
            }

            if(right->nodetype == NODE_NULL || right->nodetype == NODE_ALLOC) {
                typeTable* leftType = getType(left)->type;
                if(leftType != ttLookup("int") && leftType != ttLookup("str")) {
                    // Allowed
                    return;
                }
            }
            
            if(left->nodetype == NODE_ADDR_OF) {
                printf("Semantics Error: Cannot assign to address of variable '%s'\n", left->right->varname);
                exit(1);
            }

            if(left->nodetype == NODE_ID) {

                // ID nodes that are pointers
                gst* entry = globalLookup(left->varname);
                if(entry && entry->ptr_level > 0) {
                    if(right->nodetype == NODE_NULL) {
                        // Allowed
                        break;
                    }
                }
            }


            //General check for cases where LHS is ID or ARRAY or PTR but RHS is not NULL.
            if(getDerefLevel(left) != getDerefLevel(right)) {
                printf("Semantics Error: Dereference level mismatch in assignment to variable '%s'\n", left->varname);
                exit(1);
            }
            if(getType(left)->type != getType(right)->type || getType(left)->cType != getType(right)->cType) {
                printf("Semantics Error: type mismatch in assignment to variable '%s' ('%s' vs '%s')\n", left->varname, getType(left)->type ? getType(left)->type->name : "unknown", getType(right)->type ? getType(right)->type->name : "unknown");
                exit(1);
            }

            return;
        }

        case NODE_ID: {
            gst* entry = globalLookup(root->varname);
            if(entry == NULL) {
                printf("Semantics Error: Undeclared variable '%s'\n", root->varname);
                exit(1);
            }
            if(root->type == NULL) {
                root->type = entry->type;
            }
            if(root->cType == NULL) {
                root->cType = entry->cType;
            }
            return;
        }

        case NODE_ADDR_OF: {
            if(!root->gstEntry) {
                root->gstEntry = globalLookup(root->varname);
                if(root->gstEntry == NULL) {
                    printf("Semantics Error: Undeclared variable '%s'\n", root->varname);
                    exit(1);
                }
                root->type = root->gstEntry->type;
            }
            return;
        }

        case NODE_ARRAY: {
            // Check index expression semantics
            semantics(root->right); 

            if(root->gstEntry == NULL) {
                printf("Semantics Error: Undeclared array '%s'\n", root->varname);
                exit(1);
            }
            if(root->gstEntry->size == 1) {
                printf("Semantics Error: Variable '%s' is not declared as an array \n", root->varname);
                exit(1);
            }
            if(getDerefLevel(root->right) != 0) {
                printf("Semantics Error: Invalid dereference level in index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(getType(root->right)->type != ttLookup("int")) {
                printf("Semantics Error: Non-integer index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Semantics Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
            return;
        }

        case NODE_PTR: {
            gst* entry = globalLookup(root->varname);
            if(!entry) {
                printf("Semantics Error: Undeclared pointer variable '%s'\n", root->varname);
                exit(1);
            }
            if(root->type == NULL) {
                root->type = entry->type;
            }
            if(globalLookup(root->right->varname) == NULL) {
                printf("Semantics Error: Undeclared pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            if(getDerefLevel(root) < 0) {
                printf("Semantics Error: Invalid dereference level for pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            return;
        }

        case NODE_EQ:
        case NODE_NEQ: {
            semantics(root->left);
            semantics(root->right);

            typeHandle* leftType = getType(root->left);
            typeHandle* rightType = getType(root->right);

            if(root->right->nodetype == NODE_NULL) {
                if(leftType->type != ttLookup("int") && leftType->type != ttLookup("str")){
                    // Allowed
                    return;
                }
                if(getDerefLevel(root->left) > 0) {
                    // Allowed
                    return;
                }
            }

            if(getDerefLevel(root->left) != getDerefLevel(root->right)) {
                printf("Semantics Error: Dereference level mismatch in equality comparison\n");
                exit(1);
            }
            if(leftType->type != rightType->type || leftType->cType != rightType->cType) {
                printf("Semantics Error: Type mismatch in equality comparison b/w '%s' and '%s'\n", root->left->varname, root->right->varname);
                exit(1);
            }

            return;
        }

        case NODE_PLUS:
        case NODE_MINUS: {
            node* left = root->left;
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(
               getType(left)->type == ttLookup("int") && getDerefLevel(left) == 0
                &&
               getType(right)->type == ttLookup("int") && getDerefLevel(right) == 0
            ) {
                // Allowed
                root->type = ttLookup("int");
                return;
            }

            if (root->nodetype == NODE_MINUS && getDerefLevel(right) > 0) {
                printf("Semantics Error: Cannot subtract a pointer from an integer\n");
                exit(1);
            }

            if(getDerefLevel(left) > 0 && getDerefLevel(right) == 0
                &&
               getType(right)->type == ttLookup("int") 
            ) {
                // Allowed (pointer arithmetic)
                root->type = left->type;
                return;
            }

            if(getDerefLevel(right) > 0 && getDerefLevel(left) == 0
                &&
               getType(left)->type == ttLookup("int") 
            ) {
                // Allowed (pointer arithmetic)
                root->type = right->type;
                return;
            }

            printf("Semantics Error: Invalid operands for operator '%s'\n", root->nodetype == NODE_PLUS ? "+" : "-");
            exit(1);
        }

        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD: {
            node* left = root->left;
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(
               getType(left)->type == ttLookup("int") && getDerefLevel(left) == 0
                &&
               getType(right)->type == ttLookup("int") && getDerefLevel(right) == 0
            ) {
                // Allowed
                root->type = ttLookup("int");
                return;
            }

            printf("Semantics Error: Invalid operands for operator '%s'\n", root->nodetype == NODE_MUL ? "*" : (root->nodetype == NODE_DIV ? "/" : "%"));
            exit(1);
        }
        case NODE_GT:
        case NODE_GTE:
        case NODE_LT:
        case NODE_LTE: {
            node* left = root->left;
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(
               getType(left)->type == ttLookup("int") && getDerefLevel(left) == 0
                &&
               getType(right)->type == ttLookup("int") && getDerefLevel(right) == 0
            ) {
                // Allowed
                root->type = ttLookup("bool");
                return;
            }

            printf("Semantics Error: Invalid operands for OP node");
            exit(1);
        }

        case NODE_IF: 
        case NODE_IFELSE: {
            semantics(root->left); // condition
            semantics(root->right); // if body

            if(getType(root->left)->type != ttLookup("bool")) {
                printf("Semantics Error: Condition expression in if statement must be of type bool\n");
                exit(1);
            }

            return;
        }

        case NODE_WHILE:
        case NODE_DOWHILE: {
            node *left = root->left; // condition
            node *right = root->right; // body

            semantics(left);
            semantics(right); 

            if(getType(left)->type != ttLookup("bool")) {
                printf("Semantics Error: Condition expression in while statement must be of type bool\n");
                exit(1);
            }

            return;
        }

        case NODE_FNDECL: {
            return;
        }

        case NODE_FNDEF: {
            gst* gstEntry = root->gstEntry;
            node* paramList = root->left;

            if(!gstLookup(root->varname)) {
                printf("Semantics Error: Function '%s' not declared\n", root->varname);
                exit(1);
            }

            if(root->type != root->gstEntry->type || root->cType != root->gstEntry->cType) {
                printf("Semantics Error: Return type mismatch in function definition for function '%s'\n", root->varname);
                exit(1);
            }

            struct paramStruct* params = gstEntry->paramList;
            matchParamsList(&params, paramList);
            if(params != NULL) {
                printf("Semantics Error: Too few arguments in function definition for function '%s'\n", root->varname);
                exit(1);
            }

            // Build LST (Includes semantic checks for redeclaration of local variables and parameters)
            params = gstEntry->paramList; // reset params pointer to head of list
            while(params != NULL) {
                lstInstall(params->name, params->type, params->cType, params->ptr_level);
                params = params->next;
            }
            bindParams(); // assign bindings to parameters

            buildLST(root->right->left, gstEntry->paramList);

            // printGST(lstRoot);

            semantics(root->right->right); // function body

            freeLst();
            return;
        }

        
        case NODE_FIELD: {
            fieldLevel++;
            if(fieldLevel <= 1) {
                enforcePrivateAttributes(root);
            }
            if(root->left->nodetype != NODE_FIELD) {
                if(getDerefLevel(root->left) != 0) {
                    printf("Semantics Error: Cannot access field of pointer variable '%s' without dereferencing\n", root->left->varname);
                    exit(1);
                }
                if(root->left->nodetype == NODE_ADDR_OF) {
                    printf("Semantics Error: Cannot access field of address of variable '%s'\n", root->left->right->varname);
                    exit(1);
                }
            }
            semantics(root->left);
            fieldLevel--;
            return;
        }

        case NODE_FIELDFN: {
            if(root->left->nodetype != NODE_FIELD) {
                if(getDerefLevel(root->left) != 0) {
                    printf("Semantics Error: Cannot access field of pointer variable '%s' without dereferencing\n", root->left->varname);
                    exit(1);
                }
                if(root->left->nodetype == NODE_ADDR_OF) {
                    printf("Semantics Error: Cannot access field of address of variable '%s'\n", root->left->right->varname);
                    exit(1);
                }
            }
            semantics(root->left);
            return;
        }

        case NODE_FNCALL: {
            gst* gstEntry = root->gstEntry;
            node* argList = root->right;

            if(!gstEntry) {
                printf("Semantics Error: Undeclared function '%s'\n", root->varname);
                exit(1);
            }

            if(root->type != gstEntry->type || root->cType != gstEntry->cType) {
                printf("Semantics Error: Return type mismatch in function call for function '%s'\n", root->varname);
                exit(1);
            }

            semantics(argList); // resolve/check argument expression types before signature matching

            struct paramStruct* params = gstEntry->paramList;
            matchParamsList(&params, argList);

            if(params != NULL) {
                printf("Semantics Error: Too few arguments in function call for function '%s'\n", root->varname);
                exit(1);
            }

            return;
        }

        case NODE_DECL:
        case NODE_LDECL: {
            if(ttLookup(root->left->varname) == NULL && ctLookup(root->left->varname) == NULL) {
                printf("Semantics Error: Undeclared type '%s' used in declaration\n", root->left->varname);
                exit(1);
            }
            // semantics(root->right); // process the varlist
            return;
        }

        case NODE_TYPEDEF: {
            // semantics taken care during AST generation
            return;
        }

        case NODE_MAIN: {
            buildLST(root->left, NULL);
            // printGST(lstRoot);
            semantics(root->right);
            freeLst();
            return;
        }

        case NODE_INITIALIZE: 
        case NODE_ALLOC:{
            return;
        }

        case NODE_FREE: {
            semantics(root->left);
            typeHandle* argType = getType(root->left);
            if(argType == NULL) {
                printf("Semantics Error: Attempting to free variable of unknown type\n");
                exit(1);
            }
            if(argType->type == ttLookup("int") || argType->type == ttLookup("str")) {
                printf("Semantics Error: Cannot free variable of type '%s'\n", argType->type ? argType->type->name : argType->cType->name);
                exit(1);
            }
            return;
        }

        case NODE_CDEF: {
            currClass = ctLookup(root->varname);
            semantics(root->right->right); // class fn definition
            currClass = NULL;
            return;
        }

        case NODE_PARAM: {
            if(getType(root->left) == NULL) {
                printf("Semantics Error: Undeclared type for parameter '%s'\n", root->right->varname);
                exit(1);
            }
            if(strcmp(root->right->varname, "self") == 0 && currClass != NULL) {
                // Allowed
            }
            return;
        }

        case NODE_CFNDEF: {
            cMethodList* methodEntry = ctMethodLookup(currClass->name, root->varname);

            if(methodEntry == NULL) {
                printf("Semantics Error: Method '%s' not declared in class '%s'\n", root->varname, currClass->name);
                exit(1);
            }
            
            if(root->type != methodEntry->type) {
                printf("Semantics Error: Return type mismatch in method definition for method '%s' in class '%s'\n", root->varname, currClass->name);
                exit(1);
            }

            struct paramStruct* params = methodEntry->params;
            node* paramList = root->left;
            matchParamsList(&params, paramList);
            if(params != NULL) {
                printf("Semantics Error: Too few arguments in method definition for method '%s' in class '%s'\n", root->varname, currClass->name);
                exit(1);
            }

            // Build LST (Includes semantic checks for redeclaration of local variables and parameters)
            params = methodEntry->params; // reset params pointer to head of list
            lstInstall("self", NULL, ctLookup(currClass->name), 0); // install self parameter
            while(params != NULL) {
                lstInstall(params->name, params->type, params->cType, params->ptr_level);
                params = params->next;
            }
            bindParams(); // assign bindings to parameters
            
            buildLST(root->right->left, methodEntry->params);

            printGST(lstRoot);

            semantics(root->right->right); // method body

            freeLst();
            return;
        }

        case NODE_NEW: {
            // cType already attched in tree.c
            if(root->cType == NULL) {
                printf("Semantics Error: Attempting to instantiate unknown class '%s'\n", root->varname);
                exit(1);
            }
            return;
        }
    }

    semantics(root->left);
    semantics(root->right);
}