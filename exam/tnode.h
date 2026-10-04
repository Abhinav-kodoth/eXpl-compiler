#ifndef TNODE_H
#define TNODE_H

typedef struct tnode {
    int val;
    char *varname;
    char *op;
    struct tnode *left, *right;
} tnode;

#define YYSTYPE tnode*

struct tnode* makeNumNode(int n);
struct tnode* makeIdNode(char *name);
struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r);
struct tnode* makeAssignNode(struct tnode *id, struct tnode *expr);

void printAST(struct tnode *root, int depth);

#endif