#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tnode.h"

struct tnode* createTree(int val, int nodetype, char *varname, struct tnode *l, struct tnode *r) {
    struct tnode *t = (struct tnode*) malloc(sizeof(struct tnode));
    t->val = val;
    t->type = 0;
    t->nodetype = nodetype;
    t->varname = varname ? strdup(varname) : NULL;
    t->left = l;
    t->right = r;
    return t;
}

static const char* nodeTypeName(int nt) {
    switch (nt) {
        case NUM_T:       return "NUM";
        case ID_T:        return "ID";
        case PLUS_T:       return "+";
        case MINUS_T:      return "-";
        case MUL_T:        return "*";
        case DIV_T:        return "/";
        case ASSIGN_T:     return "=";
        case READ_T:       return "READ";
        case WRITE_T:      return "WRITE";
        case CONNECTOR_T:  return "CONNECTOR";
        default:           return "?";
    }
}

void printAST(struct tnode *root, int depth) {
    if (root == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");

    if (root->nodetype == NUM_T)
        printf("NUM(%d)\n", root->val);
    else if (root->nodetype == ID_T)
        printf("ID(%s)\n", root->varname);
    else
        printf("%s\n", nodeTypeName(root->nodetype));

    printAST(root->left, depth + 1);
    printAST(root->right, depth + 1);
}