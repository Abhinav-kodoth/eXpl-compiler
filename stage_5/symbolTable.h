#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_BASE 4096

extern int localBinding ;

struct ParamStruct {
    char * name ;
    int type ;
    struct ParamStruct * next ;
};

struct Gsymbol {
    char *name ;       // name of the variable
    int type ;         // type of the variable
    int size1 ;        // size of the type of the variable
    int binding ;      // stores the static memory address allocated to the variable
    int flabel ;
    struct tnode * bodyAST ;
    struct ParamStruct * paramList ;
    struct Lsymbol * Lentry ;
    struct Gsymbol *next ;
};

struct Lsymbol {
    char *name ;              //name of the variable
    int type ;                //type of the variable
    int binding ;             //local binding of the variable
    struct Lsymbol *next ;    //points to the next Local Symbol Table entry
};

void checkDeclAndDefn(struct Gsymbol * currentFunc,int type);

void compareParamList(struct Gsymbol * currentFunc,struct ParamStruct * currParamList);

struct Lsymbol *createLocalSymbolNode(char * name ,int type);

struct Lsymbol *createLSTFromParams(struct ParamStruct * paramList);

struct Lsymbol *findLocalSymbol(struct Lsymbol * head, char * name);

struct Lsymbol *insertToLocalTable(struct Lsymbol * head, char * name, int type);

struct Gsymbol* createSymbolNode(char* name, int type, int size1, int flabel);

struct ParamStruct * insertToParamList(struct ParamStruct * head,char * name, int type );

struct ParamStruct * createParamStructNode(char * name, int type);

struct ParamStruct * findinParamList(struct ParamStruct * head, char * name);

// Returns a pointer to the symbol table entry for the variable, returns NULL otherwise.
struct Gsymbol * find(struct Gsymbol * head ,char * name); 

//Creates a symbol table entry.
struct Gsymbol * insertTable(struct Gsymbol * head , char *name, int type, int size1, int flabel); 

int countParameterList(struct ParamStruct * paramlist);

void printSymbolTable(struct Gsymbol * head);

void printLocalSymbolTable(struct Lsymbol * head);

void printParamList(struct ParamStruct * head) ;

void insertParamListToGST(struct Gsymbol * head, char * name, struct ParamStruct * paramList);