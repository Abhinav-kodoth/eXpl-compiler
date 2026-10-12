#ifndef TNODE_H
#define TNODE_H

typedef struct tnode {
    int val;
    char *op;
    struct tnode *left, *right;
} tnode;

#define YYSTYPE tnode*

struct tnode* makeLeafNode(int n);
struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r);

void printPrefix(struct tnode *root);
void printPostfix(struct tnode *root);

#endif