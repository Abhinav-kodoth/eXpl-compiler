#include "exptree.h"
#include "symbolTable.h"

struct tnode * createTreeNode(int val, int type, char* varname, int nodetype, struct Gsymbol * Gentry,struct tnode *l, struct tnode * m,struct tnode *r) {
    struct tnode * temp = (struct tnode * ) malloc(sizeof(struct tnode));

    temp->left = l ;
    temp->right = r ;
    temp->mid = m ;

    temp->Gentry = Gentry ;
    temp->type = type ;
    temp->nodetype = nodetype ;

    temp->val = val ; 
    temp->varname = varname ;

    return temp ;
}

struct tnode * connectNode(struct tnode * left, struct tnode * right) {

    return createTreeNode(NO_VAL,NO_TYPE,NULL,CONNECT_NODE,NULL,left,NULL,right);
}

struct tnode * ipOpNode(int nodetype ,struct tnode * left) {
    //Will only have left side ID present since its read or write node
    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,left,NULL,NULL);
}

struct tnode * loopNode(int nodetype, struct tnode * left, struct tnode * right) {

    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,left,NULL,right);
}

struct tnode * flowControlNode(int nodetype) {
    //Continue and Break Stmt node
    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,NULL,NULL,NULL);
}

int isArithmeticOp(int nodeType) {
    return (nodeType == ADD_NODE || nodeType == MINUS_NODE ||
            nodeType == MUL_NODE || nodeType == DIV_NODE);
}

int isRelop(int nodeType) {
    return (nodeType == LT_NODE || nodeType == LE_NODE ||
            nodeType == GT_NODE || nodeType == GE_NODE ||
            nodeType == EQ_NODE || nodeType == NE_NODE );
}

int isLogical(int nodeType) {
    return (nodeType == AND_NODE || nodeType == OR_NODE ||
            nodeType == NOT_NODE) ;
}

struct tnode * exprNode(int nodeType, struct tnode * left , struct tnode * right) {

    if(isLogical(nodeType)) {
        if(left->type != BOOL_TYPE || right->type != BOOL_TYPE) {
            printf("Type Mismatch in Bool\n");
            exit(1);
        } else {
            return createTreeNode(NO_VAL,BOOL_TYPE,NULL,nodeType,NULL,left,NULL,right);
        }
    }

    if(isArithmeticOp(nodeType)) {
        if(left->type != INT_TYPE || right->type != INT_TYPE) {
            printf("Type Mismatch in Arithmetic\n");
            exit(1);
        } else {
            return createTreeNode(NO_VAL,INT_TYPE,NULL,nodeType,NULL,left,NULL,right);
        }
    }

    if(isRelop(nodeType)) {
        if(left->type != INT_TYPE || right->type != INT_TYPE) {
            printf("Type Mismatch in Relop\n");
            exit(1);
        } else {
            return createTreeNode(NO_VAL,BOOL_TYPE,NULL,nodeType,NULL,left,NULL,right);
        }
    }

    printf("Unknown Expression\n");
    exit(1) ;
}

struct tnode * assignNode(struct tnode * left, struct tnode * right) {
    if(left->nodetype == DEREF_NODE) {
        if(left->type == INT_PTR_TYPE) {
            if(right->type != INT_TYPE) {
                printf("Type Mismatch\n");
                exit(1);
            }
        }
        if(left->type == STR_TYPE) {
            printf("Type Incompatible");
            exit(1);
        }
    }

    if(left->nodetype == ID_NODE || left->nodetype == ARR_NODE) {
        if(left->type == INT_TYPE) {
            if(right->type != INT_TYPE) {
                printf("Type Mismatch: Cannot assign non-integer to integer variable\n");
                exit(1);
            }
        }
        if(left->type == STR_TYPE) {
            if(right->type != STR_TYPE) {
                printf("Type Mismatch: Cannot assign non-string to string variable\n");
                exit(1);
            }
        }
    }
    return createTreeNode(NO_VAL,NO_TYPE,NULL,ASSIGN_NODE,NULL,left,NULL,right);
}

struct tnode * exitNode(int nodeType) {
    return createTreeNode(NO_VAL,NO_TYPE,NULL,EXIT_NODE,NULL,NULL,NULL,NULL);
}

struct tnode * ifelseNode(int nodeType ,struct tnode * left, struct tnode * mid ,struct tnode * right) {
    return  createTreeNode(NO_VAL,NO_TYPE,NULL,nodeType,NULL,left,mid,right);
}

struct tnode * setTypeId(struct Gsymbol * Ghead ,struct Lsymbol * Lhead ,struct tnode * idNode,struct tnode * leftExprNode, struct tnode * rightExprNode) {
    struct Lsymbol * Ltemp = findLocalSymbol(Lhead,idNode->varname);

    //Check Local Symbol Table first
    if(Ltemp != NULL) {
        idNode->left = leftExprNode ;
        idNode->right = rightExprNode ;
        idNode->type = Ltemp->type ;
        idNode->Lentry = Ltemp ;
        return idNode ;
    }
    struct Gsymbol * Gtemp = find(Ghead , idNode->varname) ;

    //Check global symbol table
    if(Gtemp == NULL) {
        printf("Id not declared\n");
        exit(1);
    }

    idNode->left = leftExprNode ;
    idNode->right = rightExprNode ;
    idNode->type = Gtemp->type ;
    idNode->Gentry = Gtemp ;

    return idNode ;
}

struct tnode * pointerNode(struct Gsymbol * head,int nodeType,struct tnode * node, struct tnode * left, struct tnode * right) {
    struct Gsymbol * symbolNode = find(head, node->varname);

    struct tnode * temp = createTreeNode(NO_VAL,NO_TYPE,NULL,nodeType,symbolNode,NULL,NULL,NULL);

    if(nodeType == DEREF_NODE) {
        if(node->Gentry->type == INT_PTR_TYPE) {
            temp->type = INT_TYPE;
            node->type = INT_PTR_TYPE ;
        } else {
            temp->type = STR_TYPE ;
            node->type = STR_PTR_TYPE ;
        }
    } else if(nodeType == ADDR_NODE) {
        if(node->Gentry->type == INT_TYPE) {
            temp->type = INT_PTR_TYPE ;
            node->type = INT_TYPE ;
        } else {
            temp->type = STR_PTR_TYPE ;
            node->type = STR_TYPE ;
        }
    }
    temp->left = node ;

    return temp ;
}

struct tnode * returnNode(struct Gsymbol * funcEntry, int nodetype, struct tnode * node) {
    if(funcEntry == NULL) {
        printf("Error: Return statement outside function\n");
        exit(1) ;
    }

    // Type check: return expression type must match function return type
    if(node->type != funcEntry->type) {
        printf("Error: Return type mismatch in function '%s'.", funcEntry->name);
        exit(1);
    }
    
    return createTreeNode(NO_VAL,node->type,NULL,RETURN_NODE,funcEntry,node,NULL,NULL);
}

struct tnode * funcNode(struct Gsymbol * Ghead ,struct tnode * node, struct tnode * arglist) {

    struct Gsymbol * funcEntry = find(Ghead,node->varname);

    if(funcEntry == NULL) {
        printf("Error : Function not declared and called\n");
        exit(1) ;
    }

    struct ParamStruct * paramHead = funcEntry->paramList ;

    struct tnode * argHead = arglist ;

    // Type check arguments against parameters
    while(argHead != NULL &&  paramHead != NULL) {
        if(argHead->type != paramHead->type ) {
            printf("Type mismatch for argument '%s' in function call to '%s'\n",argHead->varname,paramHead->name);
            exit(1);
        }
        paramHead = paramHead->next ;
        argHead = argHead->argList ;
    }

    if(paramHead != NULL) {
        printf("Too few arguments in function call\n");
        exit(1);
    }

    if(argHead != NULL) {
        printf("Too many arguments in function call\n");
        exit(1);
    }

    struct tnode * callNode =  createTreeNode(NO_VAL,funcEntry->type,funcEntry->name,FUNC_CALL_NODE,funcEntry,NULL,NULL,NULL);

    node->argList = arglist ;
    callNode->left = node ;

    return callNode ;
}

struct tnode * appendArg(struct tnode * argList, struct tnode * newArg) {
    newArg->argList = NULL ;

    if(argList == NULL) {
        return newArg ;
    }

    struct tnode * temp = argList ;

    while(temp->argList) {
        temp = temp->argList ;
    }
    temp->argList = newArg ;

    return argList ;
}
