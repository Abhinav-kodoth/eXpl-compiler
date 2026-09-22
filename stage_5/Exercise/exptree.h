#ifndef exptree_h
#define exptree_h

#define NO_VAL -1
#define ENTRY_CODE 0
#define EXIT_CODE 1
#define READ_CODE 2
#define WRITE_CODE 3

#define EVAL_NORMAL 0
#define EVAL_BREAK -1
#define EVAL_CONTINUE -2 

#define NA -404

#include "TypeTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum nodetype {
    NUM_NODE        = 0,
    ID_NODE         = 1,
    STR_NODE        = 2,

    READ_NODE       = 3,
    WRITE_NODE      = 4,
    ASSIGN_NODE     = 5,

    ADD_NODE        = 6,
    MINUS_NODE      = 7,
    MUL_NODE        = 8,
    DIV_NODE        = 9,
    MOD_NODE        = 10,

    EQ_NODE         = 11,
    NE_NODE         = 12,
    LT_NODE         = 13,
    LE_NODE         = 14,
    GT_NODE         = 15,
    GE_NODE         = 16,

    CONNECT_NODE    = 17,
    IF_NODE         = 18,
    ELSE_NODE       = 19,
    WHILE_NODE      = 20,
    CONTINUE_NODE   = 21,
    BREAK_NODE      = 22,
    REPEAT_NODE     = 23,
    DOWHILE_NODE    = 24,

    ARITHOP_NODE    = 25,
    RELOP_NODE      = 26,

    ARR_NODE        = 27,
    MATRIX_NODE     = 28,
    ADDR_NODE       = 29,
    DEREF_NODE      = 30,
    REF_NODE        = 31,
    EXIT_NODE       = 32,
    FUNC_CALL_NODE  = 33,
    PARAM_NODE      = 34,
    GID_NODE        = 35,
    RETURN_NODE     = 36,
    AND_NODE        = 37,
    OR_NODE         = 38,
    NOT_NODE        = 39,
    FIELD_NODE      = 40
};

// enum Datatype {
//     INT_TYPE    = 0,
//     BOOL_TYPE   = 1,
//     STR_TYPE    = 2,
//     NO_TYPE     = 3,
//     INT_PTR_TYPE = 4,
//     STR_PTR_TYPE = 5
// };

typedef struct tnode {
    int val;                      // value of a number for NUM nodes.
    char * varname;               // name of a variable for ID nodes
    int nodetype;                 // information about non-leaf nodes - read/write/connector/+/* etc.
    bool isPtr ;                  // flag to check whether variable is a pointer or not
    struct TypeTable * type ;     // pointer to the Typetable entry of variable type
    struct Gsymbol * Gentry ;     // pointer to GST entry for global variables and functions
    struct Lsymbol * Lentry ;     // pointer to LST entry for local variables and functions
    struct tnode * argList ;      // pointer to the expression list given as arguments to a function call
    struct tnode * left, *right, *mid ;
}tnode;

struct tnode * createTreeNode(int isPtr,int val, struct TypeTable * type, char* varname, int nodetype, struct Gsymbol * Gentry ,struct tnode *l,struct tnode * m, struct tnode *r);
struct tnode * connectNode(struct tnode * left, struct tnode * right);
struct tnode * ipOpNode(int nodetype ,struct tnode * left);
struct tnode * loopNode(int nodetype , struct tnode * left, struct tnode * right);
struct tnode * flowControlNode(int nodetype);
struct tnode * exprNode(int nodetype ,struct tnode * left, struct tnode * right);
struct tnode * assignNode(struct tnode * left, struct tnode * right);
struct tnode * setTypeId(struct Gsymbol * Ghead,struct Lsymbol * Lhead, struct tnode * idNode,struct tnode * leftExprNode, struct tnode * rightExprNode);
struct tnode * ifelseNode(int nodeType, struct tnode * left, struct tnode * mid,struct tnode * right);
struct tnode * pointerNode(struct Gsymbol * head,int nodeType,struct tnode * node,struct tnode * left, struct tnode * right);
struct tnode * exitNode(int nodeType);
struct tnode * funcNode(struct Gsymbol * Ghead, struct tnode * node, struct tnode * arglist);
struct tnode * appendArg(struct tnode * argList, struct tnode * newArg);
struct tnode * returnNode(struct Gsymbol * funcEntry ,int nodetype,struct tnode * node);
struct tnode * fieldNode(struct Fieldlist *, int fieldIndex, struct TypeTable * type, char * name, int nodetype, struct tnode * rightExprNode);

#endif
