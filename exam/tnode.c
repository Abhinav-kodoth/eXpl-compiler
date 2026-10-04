
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tnode.h"

static struct tnode* newNode(void) {
    struct tnode *t = malloc(sizeof(struct tnode));
    if (t == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    t->val = 0;
    t->varname = NULL;
    t->op = NULL;
    t->left = NULL;
    t->right = NULL;
    return t;
}

struct tnode* makeNumNode(int n) {
    struct tnode *t = newNode();
    t->val = n;
    return t;
}

struct tnode* makeIdNode(char *name) {
    struct tnode *t = newNode();
    t->varname = strdup(name);
    return t;
}

struct tnode* makeOperatorNode(char op, struct tnode *l, struct tnode *r) {
    struct tnode *t = newNode();
    t->op = malloc(2);
    if (t->op == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    t->op[0] = op;
    t->op[1] = '\0';
    t->left = l;
    t->right = r;
    return t;
}

struct tnode* makeAssignNode(struct tnode *id, struct tnode *expr) {
    struct tnode *t = newNode();
    t->op = strdup("=");
    t->left = id;
    t->right = expr;
    return t;
}

void printAST(struct tnode *root, int depth) {
    if (root == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");

    if (root->varname != NULL)
        printf("ID(%s)\n", root->varname);
    else if (root->op == NULL)
        printf("NUM(%d)\n", root->val);
    else
        printf("%s\n", root->op);

    printAST(root->left, depth + 1);
    printAST(root->right, depth + 1);
}