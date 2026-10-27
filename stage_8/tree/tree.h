#ifndef TREE_H
#define TREE_H

typedef struct gst gst; // Forward declaration of gst from gst.h
typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct fieldList fieldList; // Forward declaration of fieldList from tt.h


typedef enum {
    NODE_EMPTY,
    NODE_CONNECTOR,
    NODE_READ,
    NODE_WRITE,
    NODE_NUM,
    NODE_STR,
    NODE_ID,
    NODE_PTR,
    NODE_ADDR_OF,
    NODE_NULL,
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
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
    NODE_DOWHILE,
    NODE_TYPE,
    NODE_DECL,
    NODE_ARRAY,
    NODE_FNDECL,
    NODE_FNDEF,
    NODE_FNCALL,
    NODE_PARAM,
    NODE_RETURN,
    NODE_MAIN,
    NODE_LDECL,
    NODE_ARG,
    NODE_TYPEDEF,
    NODE_FIELDDECL,
    NODE_FIELD,
    NODE_INITIALIZE,
    NODE_FREE,
    NODE_ALLOC,
    NODE_FIELDFN,
    NODE_CFNDEF,
    NODE_CMETHOD,
    NODE_CFIELD,
    NODE_CDEF,
    NODE_NEW,
    NODE_EXTENDS
} nodeType;

typedef enum varType varType; // Forward declaration of varType for gst.h
typedef struct classTable classTable; // Forward declaration of classTable for gst.h

#include "../symbol_table/gst.h"

typedef struct node{
    int val;        
    char* strval;    
    struct typeTable* type; 
    struct classTable* cType;
    char* varname;  
    nodeType nodetype;   
    struct gst* gstEntry;
    struct node *left, *right; 
} node;

int getDerefLevel(node* root);

node* makeLeafNumNode(int n);
node* makeLeafStrNode(char* str);
node* makeLeafIdNode(char* varname);
node *makeWriteNode(node* left);
node* makeReadNode(node *left);
node* makeOpNode(char* op, node* left, node* right);
node* makeConnectorNode(node* left, node* right);
node* makeIfNode(node* cond, node* ifStmt);
node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt);
node* makeWhileNode(node* cond, node* body);
node* makeBreakNode();
node* makeContinueNode();
node* makeBrkpNode();
node* makeDoWhileNode(node* body, node* cond);
node* makeDeclNode(node* type, node* varlist);
node* makeTypeNode(char* type);
node* makeArrayNode(node* varname, node* size);
node* makePtrNode(node* ptrTo);
node* makeAddressNode(node* var);
node* makeNullNode();
node* makeFnDeclNode(node* name, node* paramList);
node* makeFnDefNode(node* type, node* name, node* paramList, node* lDeclBlock, node* body);
node* makeParamNode(node* type, node* var);
node* makeReturnNode(node* retVal);
node* makeFnCallNode(node* fnName, node* argList);
node* makeMainNode(node* lDeclBlock, node* body);
node* makeLDeclNode(node* type, node* varlist);
node* makeArgNode(node* expr);
node* makeTypeDefNode(node* typeName, node* fieldList);
node* makeFieldDeclNode(node* typeName, node* fieldName);
node* makeInitializeNode();
node* makeFreeNode(node* arg);
node* makeAllocNode();
node* makeFieldNode(node* var, node* field);
node* makeFieldFnNode(node* var, node* field, node* args);
node* makeClassFnDefNode(node* fnDef);
node* makeClassMethodNode(node* type, node* name, node* params);
node* makeClassFieldNode(node* type, node* name);
node* makeClassDefNode(node* name, node* fieldList, node* methodList, node* methodDefList);
node* makeInheritedClassDefNode(node* parentClassName, node* name, node* fieldList, node* methodList, node* methodDefList);
node* makeNewNode(node* className);
#endif