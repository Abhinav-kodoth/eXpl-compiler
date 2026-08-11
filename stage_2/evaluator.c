#include <stdio.h>
#include <stdlib.h>
#include "tnode.h"
#include "evaluator.h"

static int vars[26];

static int evalExpr(struct tnode *t) {
    if (t == NULL) {
        fprintf(stderr, "Error: evalExpr called on NULL node\n");
        exit(1);
    }
    switch (t->nodetype) {
        case NUM_T: return t->val;
        case ID_T:  return vars[t->varname[0] - 'a'];
        case PLUS_T:  return evalExpr(t->left) + evalExpr(t->right);
        case MINUS_T: return evalExpr(t->left) - evalExpr(t->right);
        case MUL_T:   return evalExpr(t->left) * evalExpr(t->right);
        case DIV_T:   return evalExpr(t->left) / evalExpr(t->right);
        default:
            fprintf(stderr, "Error: evalExpr called on a non-expression node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

static void evalStmt(struct tnode *t) {
    if (t == NULL) return;
    switch (t->nodetype) {
        case CONNECTOR_T:
            evalStmt(t->left);
            evalStmt(t->right);
            break;
        case ASSIGN_T:
            vars[t->left->varname[0] - 'a'] = evalExpr(t->right);
            break;
        case READ_T:
            scanf("%d", &vars[t->left->varname[0] - 'a']);
            break;
        case WRITE_T:
            printf("%d\n", evalExpr(t->left));
            break;
        default:
            fprintf(stderr, "Error: evalStmt called on a non-statement node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

void evaluate(struct tnode *root) {
    evalStmt(root);
}