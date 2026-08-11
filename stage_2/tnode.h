#ifndef TNODE_H
#define TNODE_H

typedef struct tnode {
    int val;
    int type;
    char *varname;
    int nodetype;
    struct tnode *left, *right;
} tnode;

#define YYSTYPE tnode*

#define NUM_T        1
#define ID_T         2
#define PLUS_T       3
#define MINUS_T      4
#define MUL_T        5
#define DIV_T        6
#define ASSIGN_T     7
#define READ_T       8
#define WRITE_T      9
#define CONNECTOR_T  10

struct tnode* createTree(int val, int nodetype, char *varname, struct tnode *l, struct tnode *r);
void printAST(struct tnode *root, int depth);

#endif