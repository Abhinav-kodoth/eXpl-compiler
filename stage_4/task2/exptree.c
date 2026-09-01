#include "exptree.h"
#include "symbolTable.h"

struct tnode * createTreeNode(int val, int type, char* varname, int nodetype, struct Gsymbol * Gentry,struct tnode *l, struct tnode *r) {
    struct tnode * temp = (struct tnode * ) malloc(sizeof(struct tnode));

    temp->left = l ;
    temp->right = r ;

    temp->Gentry = Gentry ;
    temp->type = type ;
    temp->nodetype = nodetype ;

    temp->val = val ; 
    temp->varname = varname ;

    return temp ;
}

struct tnode * connectNode(struct tnode * left, struct tnode * right) {

    return createTreeNode(NO_VAL,NO_TYPE,NULL,CONNECT_NODE,NULL,left,right);
}

struct tnode * ipOpNode(int nodetype ,struct tnode * left) {
    //Will only have left side ID present since its read or write node
    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,left,NULL);
}

struct tnode * loopNode(int nodetype, struct tnode * left, struct tnode * right) {

    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,left,right);
}

struct tnode * flowControlNode(int nodetype) {
    //Continue and Break Stmt node
    return createTreeNode(NO_VAL,NO_TYPE,NULL,nodetype,NULL,NULL,NULL);
}

int isArithmeticOp(int nodeType) {
    return (nodeType == ADD_NODE || nodeType == MINUS_NODE ||
            nodeType == MUL_NODE || nodeType == DIV_NODE ||
            nodeType == MOD_NODE);
}

int isRelop(int nodeType) {
    return (nodeType == LT_NODE || nodeType == LE_NODE ||
            nodeType == GT_NODE || nodeType == GE_NODE ||
            nodeType == EQ_NODE || nodeType == NE_NODE);
}


struct tnode * exprNode(int nodeType, struct tnode * left , struct tnode * right) {

    if(isArithmeticOp(nodeType)) {
        if(left->type != INT_TYPE && right->type != INT_TYPE) {
            printf("Type Mismatch\n");
            exit(1);
        } else {
            return createTreeNode(NO_VAL,INT_TYPE,NULL,nodeType,NULL,left,right);
        }
    }

    if(isRelop(nodeType)) {
        if(left->type != INT_TYPE && right->type != INT_TYPE) {
            printf("Type Mismatch\n");
            exit(1);
        } else {
            return createTreeNode(NO_VAL,BOOL_TYPE,NULL,nodeType,NULL,left,right);
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

    if(left->nodetype == ID_NODE) {
        if(left->type == INT_TYPE) {
            if(right->type != INT_TYPE) {
                printf("Type Mismatch\n");
                exit(1);
            }
        }
        if(left->type == STR_TYPE) {
            if(right->type != STR_TYPE) {
                printf("Type Mismatch\n");
                exit(1);
            }
        }
    }
    return createTreeNode(NO_VAL,NO_TYPE,NULL,ASSIGN_NODE,NULL,left,right);
}

struct tnode * exitNode(int nodeType) {
    return createTreeNode(NO_VAL,NO_TYPE,NULL,EXIT_NODE,NULL,NULL,NULL);
}

struct tnode * ifelseNode(int nodeType ,struct tnode * left, struct tnode * right) {

    return  createTreeNode(NO_VAL,NO_TYPE,NULL,nodeType,NULL,left,right);
}

struct tnode * setTypeId(struct Gsymbol * head ,struct tnode * idNode,struct tnode * leftExprNode, struct tnode * rightExprNode) {
    struct Gsymbol * temp = find(head , idNode->varname) ;

    if(temp == NULL) {
        printf("Id not declared\n");
        exit(1);
    }

    idNode->left = leftExprNode ;
    idNode->right = rightExprNode ;
    idNode->type = temp->type ;
    idNode->Gentry = temp ;

    return idNode ;
}

struct tnode * pointerNode(struct Gsymbol * head,int nodeType,struct tnode * node, struct tnode * left, struct tnode * right) {
    struct Gsymbol * symbolNode = find(head, node->varname);

    struct tnode * temp = createTreeNode(NO_VAL,NO_TYPE,NULL,nodeType,symbolNode,NULL,NULL);

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
