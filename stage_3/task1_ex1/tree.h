#ifndef TREE_H
#define TREE_H

typedef enum {
    NODE_EMPTY,
    NODE_CONNECTOR,
    NODE_READ,
    NODE_WRITE,
    NODE_NUM,
    NODE_ID,
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_ASSIGN,
    NODE_GT,
    NODE_GTE,
    NODE_LT,
    NODE_LTE,
    NODE_EQ,
    NODE_NEQ,
    NODE_IFELSE,
    NODE_IF,
    NODE_WHILE,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_BRKP,
    NODE_DOWHILE
} nodeType;

typedef enum {
    TYPE_INT,
    TYPE_BOOL
} varType;

typedef struct node {
    int val;            // value of a number for NUM nodes.
    varType type;        // type of variable
    char* varname;       // name of a variable for ID nodes
    nodeType nodetype;    // information about non-leaf nodes - read/write/connector/+/* etc.
    struct node *left, *right;  // left and right branches
} node;

node* makeLeafNumNode(int n);
node* makeLeafIdNode(char* varname);
node *makeWriteNode(node* left);
node* makeReadNode(node *left);
node* makeOpNode(char* op, node* left, node* right);
/* NOT in the pasted header (which declares NODE_ASSIGN in the enum but no
 * matching constructor) -- added here since assignment needs somewhere to
 * live. If your senior's actual tree.c instead folds "=" into makeOpNode,
 * swap this out accordingly. */
node* makeAssignNode(node* left, node* right);
node* makeConnectorNode(node* left, node* right);
node* makeIfNode(node* cond, node* ifStmt);
node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt);
node* makeWhileNode(node* cond, node* body);
node* makeBreakNode();
node* makeContinueNode();
node* makeBrkpNode();
node* makeDoWhileNode(node* body, node* cond);
/* NOT in the pasted header either (NODE_EMPTY has no listed constructor) --
 * added for the empty-program case ("begin end;"). */
node* makeEmptyNode();

/* Not part of the original interface, but needed to visually verify
 * Task 1 (AST construction) the same way we did for Stage 2. */
void printTree(node *root, int depth);

#define YYSTYPE node*

#endif