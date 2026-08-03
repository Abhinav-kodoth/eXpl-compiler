#include <stdlib.h>
#include "tnode.h"

struct tnode* makeLeafNode(int n){
    struct tnode *t=(struct tnode*)malloc(sizeof(struct tnode));
    t->val = n;
    t->op = NULL;
    t->left = NULL;
    t->right = NULL;
    return t;
}

struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r) {
    struct tnode *t = (struct tnode*) malloc(sizeof(struct tnode));
    t->op = (char*) malloc(2 * sizeof(char));
    t->op[0] = op;
    t->op[1] = '\0';
    t->left = l;
    t->right = r;
    return t;
}
