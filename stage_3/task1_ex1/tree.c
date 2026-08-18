#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

static void typeError(const char *msg) {
    fprintf(stderr, "Error: type mismatch - %s\n", msg);
    exit(1);
}

static node* allocNode(nodeType nt) {
    node *n = (node*) malloc(sizeof(node));
    n->val = 0;
    n->type = TYPE_INT;   /* default/unused for statement nodes -- the varType
                            * enum has no separate "void" tag, so this is just
                            * a harmless filler that is never inspected for
                            * statement-level nodes. */
    n->varname = NULL;
    n->nodetype = nt;
    n->left = NULL;
    n->right = NULL;
    return n;
}

node* makeLeafNumNode(int n) {
    node *t = allocNode(NODE_NUM);
    t->val = n;
    t->type = TYPE_INT;
    return t;
}

node* makeLeafIdNode(char* varname) {
    node *t = allocNode(NODE_ID);
    t->varname = strdup(varname);
    t->type = TYPE_INT;   /* every variable holds an integer, per the spec */
    return t;
}

node* makeWriteNode(node* left) {
    if (left->type != TYPE_INT)
        typeError("write() requires an integer-valued expression");
    node *t = allocNode(NODE_WRITE);
    t->left = left;
    return t;
}

node* makeReadNode(node *left) {
    /* left is expected to be an ID leaf -- no type check needed since
     * ID nodes are always TYPE_INT by construction. */
    node *t = allocNode(NODE_READ);
    t->left = left;
    return t;
}

node* makeOpNode(char* op, node* left, node* right) {
    nodeType nt;
    varType resultType;
    int isRelational = 0;

    if      (strcmp(op, "+") == 0)  { nt = NODE_PLUS;  }
    else if (strcmp(op, "-") == 0)  { nt = NODE_MINUS; }
    else if (strcmp(op, "*") == 0)  { nt = NODE_MUL;   }
    else if (strcmp(op, "/") == 0)  { nt = NODE_DIV;   }
    else if (strcmp(op, "<") == 0)  { nt = NODE_LT;   isRelational = 1; }
    else if (strcmp(op, ">") == 0)  { nt = NODE_GT;   isRelational = 1; }
    else if (strcmp(op, "<=") == 0) { nt = NODE_LTE;  isRelational = 1; }
    else if (strcmp(op, ">=") == 0) { nt = NODE_GTE;  isRelational = 1; }
    else if (strcmp(op, "==") == 0) { nt = NODE_EQ;   isRelational = 1; }
    else if (strcmp(op, "!=") == 0) { nt = NODE_NEQ;  isRelational = 1; }
    else {
        fprintf(stderr, "Error: unknown operator '%s'\n", op);
        exit(1);
    }

    if (left->type != TYPE_INT || right->type != TYPE_INT) {
        if (isRelational)
            typeError("relational operator requires integer operands");
        else
            typeError("arithmetic operator requires integer operands");
    }
    resultType = isRelational ? TYPE_BOOL : TYPE_INT;

    node *t = allocNode(nt);
    t->left = left;
    t->right = right;
    t->type = resultType;
    return t;
}

node* makeAssignNode(node* left, node* right) {
    if (right->type != TYPE_INT)
        typeError("assignment requires an integer-valued expression");
    node *t = allocNode(NODE_ASSIGN);
    t->left = left;    /* target variable (ID node) */
    t->right = right;   /* expression being assigned */
    return t;
}

node* makeConnectorNode(node* left, node* right) {
    node *t = allocNode(NODE_CONNECTOR);
    t->left = left;
    t->right = right;
    return t;
}

node* makeIfNode(node* cond, node* ifStmt) {
    if (cond->type != TYPE_BOOL)
        typeError("if guard must be a boolean expression");
    node *t = allocNode(NODE_IF);
    t->left = cond;
    t->right = ifStmt;
    return t;
}

/*
 * NODE_IFELSE needs THREE logical children (cond, ifStmt, elseStmt) but the
 * struct only has two pointers. We resolve this the same way the struct
 * forces us to for do-while/while: left = cond, and right = an internal
 * pair-node holding {ifStmt, elseStmt}. We reuse NODE_CONNECTOR as that
 * pair-node's tag purely as internal plumbing -- it is NEVER handed to a
 * generic "run left then right" evaluator/codegen path; the NODE_IFELSE
 * case always destructures it explicitly as right->left / right->right
 * (the then-branch and else-branch respectively), so there's no risk of
 * it being misinterpreted as an ordinary statement sequence.
 */
node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt) {
    if (cond->type != TYPE_BOOL)
        typeError("if guard must be a boolean expression");
    node *pair = allocNode(NODE_CONNECTOR);
    pair->left = ifStmt;
    pair->right = elseStmt;

    node *t = allocNode(NODE_IFELSE);
    t->left = cond;
    t->right = pair;
    return t;
}

node* makeWhileNode(node* cond, node* body) {
    if (cond->type != TYPE_BOOL)
        typeError("while guard must be a boolean expression");
    node *t = allocNode(NODE_WHILE);
    t->left = cond;
    t->right = body;
    return t;
}

node* makeBreakNode() {
    return allocNode(NODE_BREAK);
}

node* makeContinueNode() {
    return allocNode(NODE_CONTINUE);
}

/* NODE_BRKP: interpreted here as a source-level `brkp;` statement that
 * (in the eventual code generator) would emit the XSM BRKP instruction,
 * used to set a debugger breakpoint when running under the XSM simulator
 * with GDB (see the GDB tutorial the roadmap points to right after this
 * stage). It carries no children and has no runtime effect in the plain
 * C evaluator below beyond a visible marker. */
node* makeBrkpNode() {
    return allocNode(NODE_BRKP);
}

node* makeDoWhileNode(node* body, node* cond) {
    if (cond->type != TYPE_BOOL)
        typeError("do-while guard must be a boolean expression");
    node *t = allocNode(NODE_DOWHILE);
    t->left = body;
    t->right = cond;
    return t;
}

node* makeEmptyNode() {
    return allocNode(NODE_EMPTY);
}

/* ---------- printTree: for visually verifying Task 1 ---------- */

static const char* nodeTypeName(nodeType nt) {
    switch (nt) {
        case NODE_EMPTY:     return "EMPTY";
        case NODE_CONNECTOR: return "CONNECTOR";
        case NODE_READ:      return "READ";
        case NODE_WRITE:     return "WRITE";
        case NODE_PLUS:      return "+";
        case NODE_MINUS:     return "-";
        case NODE_MUL:       return "*";
        case NODE_DIV:       return "/";
        case NODE_ASSIGN:    return "=";
        case NODE_GT:        return ">";
        case NODE_GTE:       return ">=";
        case NODE_LT:        return "<";
        case NODE_LTE:       return "<=";
        case NODE_EQ:        return "==";
        case NODE_NEQ:       return "!=";
        case NODE_IFELSE:    return "IFELSE";
        case NODE_IF:        return "IF";
        case NODE_WHILE:      return "WHILE";
        case NODE_BREAK:      return "BREAK";
        case NODE_CONTINUE:   return "CONTINUE";
        case NODE_BRKP:       return "BRKP";
        case NODE_DOWHILE:    return "DOWHILE";
        default:              return "?";
    }
}

void printTree(node *root, int depth) {
    if (root == NULL) return;
    for (int i = 0; i < depth; i++) printf("  ");
    if (root->nodetype == NODE_NUM)
        printf("NUM(%d)\n", root->val);
    else if (root->nodetype == NODE_ID)
        printf("ID(%s)\n", root->varname);
    else
        printf("%s\n", nodeTypeName(root->nodetype));

    if (root->nodetype == NODE_IFELSE) {
        /* unwrap the internal pair-node so the printed tree shows
         * cond / then / else as three visually distinct children,
         * matching how the language actually thinks about if-else,
         * rather than exposing the CONNECTOR-tagged plumbing node. */
        printTree(root->left, depth + 1);         /* cond */
        printTree(root->right->left, depth + 1);   /* then */
        printTree(root->right->right, depth + 1);  /* else */
        return;
    }

    printTree(root->left, depth + 1);
    printTree(root->right, depth + 1);
}