#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "evaluator.h"

static int vars[26];

/* evalStmt returns a control-flow signal so break/continue can unwind
 * out of nested statement blocks until a loop catches them. */
#define SIG_NONE     0
#define SIG_BREAK    1
#define SIG_CONTINUE 2

static int evalExpr(node *t) {
    if (t == NULL) {
        fprintf(stderr, "Error: evalExpr called on NULL node\n");
        exit(1);
    }
    switch (t->nodetype) {
        case NODE_NUM: return t->val;
        case NODE_ID:  return vars[t->varname[0] - 'a'];

        case NODE_PLUS:  return evalExpr(t->left) + evalExpr(t->right);
        case NODE_MINUS: return evalExpr(t->left) - evalExpr(t->right);
        case NODE_MUL:   return evalExpr(t->left) * evalExpr(t->right);
        case NODE_DIV:   return evalExpr(t->left) / evalExpr(t->right);

        case NODE_LT:  return evalExpr(t->left) <  evalExpr(t->right);
        case NODE_GT:  return evalExpr(t->left) >  evalExpr(t->right);
        case NODE_LTE: return evalExpr(t->left) <= evalExpr(t->right);
        case NODE_GTE: return evalExpr(t->left) >= evalExpr(t->right);
        case NODE_NEQ: return evalExpr(t->left) != evalExpr(t->right);
        case NODE_EQ:  return evalExpr(t->left) == evalExpr(t->right);

        default:
            fprintf(stderr, "Error: evalExpr called on a non-expression node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

static int evalStmt(node *t) {
    if (t == NULL) return SIG_NONE;

    switch (t->nodetype) {
        case NODE_EMPTY:
            return SIG_NONE;

        case NODE_CONNECTOR: {
            int sig = evalStmt(t->left);
            if (sig != SIG_NONE) return sig;   /* break/continue: skip the rest of this block */
            return evalStmt(t->right);
        }

        case NODE_ASSIGN:
            vars[t->left->varname[0] - 'a'] = evalExpr(t->right);
            return SIG_NONE;

        case NODE_READ:
            scanf("%d", &vars[t->left->varname[0] - 'a']);
            return SIG_NONE;

        case NODE_WRITE:
            printf("%d\n", evalExpr(t->left));
            return SIG_NONE;

        case NODE_IF:
            if (evalExpr(t->left))
                return evalStmt(t->right);
            return SIG_NONE;

        case NODE_IFELSE: {
            /* t->right is the internal {then, else} pair-node built by
             * makeIfElseNode -- destructure it directly rather than
             * evaluating it as an ordinary CONNECTOR. */
            node *pair = t->right;
            if (evalExpr(t->left))
                return evalStmt(pair->left);
            else
                return evalStmt(pair->right);
        }

        case NODE_WHILE:
            while (evalExpr(t->left)) {
                int sig = evalStmt(t->right);
                if (sig == SIG_BREAK) break;
                /* SIG_CONTINUE and SIG_NONE both just fall through to
                 * re-checking the guard for the next iteration. */
            }
            return SIG_NONE;

        case NODE_DOWHILE:
            do {
                int sig = evalStmt(t->left);
                if (sig == SIG_BREAK) break;
            } while (evalExpr(t->right));
            return SIG_NONE;

        case NODE_BREAK:
            return SIG_BREAK;

        case NODE_CONTINUE:
            return SIG_CONTINUE;

        case NODE_BRKP:
            /* No runtime meaning for a plain C interpreter -- this only
             * matters once the code generator emits an XSM BRKP
             * instruction for debugging under the simulator. */
            return SIG_NONE;

        default:
            fprintf(stderr, "Error: evalStmt called on a non-statement node (nodetype=%d)\n", t->nodetype);
            exit(1);
    }
}

void evaluate(node *root) {
    evalStmt(root);
}