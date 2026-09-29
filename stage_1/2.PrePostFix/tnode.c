#include <stdio.h>
#include <stdlib.h>
#include "tnode.h"

struct tnode* makeLeafNode(int n) {
    struct tnode *t = (struct tnode*) malloc(sizeof(struct tnode));
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

void printPrefix(struct tnode *root) {
    if (root == NULL) return;
    if (root->op == NULL) {
        printf("%d ", root->val);
    } else {
        printf("%s ", root->op);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}

void printPostfix(struct tnode *root) {
    if (root == NULL) return;
    if (root->op == NULL) {
        printf("%d ", root->val);
    } else {
        printPostfix(root->left);
        printPostfix(root->right);
        printf("%s ", root->op);
    }
}