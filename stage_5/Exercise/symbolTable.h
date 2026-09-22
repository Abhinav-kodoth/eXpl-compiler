#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STACK_BASE 4096

extern int localBinding ;

struct ParamStruct {
    char * name ;
    bool isPtr ;
    struct TypeTable * type ;
    struct ParamStruct * next ;
};

struct Gsymbol {
    char *name ;                // name of the variable
    int size1 ;                 // size of the type of the variable
    int binding ;               // stores the static memory address allocated to the variable
    int flabel ;
    bool isPtr ;
    struct TypeTable * type ;   
    struct tnode * bodyAST ;
    struct ParamStruct * paramList ;
    struct Lsymbol * Lentry ;
    struct Gsymbol *next ;
};

struct Lsymbol {
    char *name ;              //name of the variable
    int size ;                // size of the variable
    bool isPtr ;              //flag to check whether the type is pointer or not
    struct TypeTable * type ; //pointer to type table entry of parameter type
    int binding ;             //local binding of the variable
    struct Lsymbol *next ;    //points to the next Local Symbol Table entry
};

void checkDeclAndDefn(struct Gsymbol * currentFunc,struct TypeTable * type);
void compareParamList(struct Gsymbol * currentFunc,struct ParamStruct * currParamList);

struct Lsymbol *createLocalSymbolNode(char * name ,struct TypeTable * type,bool isPtr,int size);
struct Lsymbol *createLSTFromParams(struct ParamStruct * paramList);
struct Lsymbol *findLocalSymbol(struct Lsymbol * head, char * name);
struct Lsymbol *insertToLocalTable(struct Lsymbol * head, char * name, struct TypeTable * type, bool isPtr, int size);

//Creates a symbol table entry.
struct Gsymbol * insertTable(struct Gsymbol * head , char *name, struct TypeTable * type,bool isPtr, int size1, int flabel); 
struct Gsymbol * find(struct Gsymbol * head ,char * name); 
struct Gsymbol* createSymbolNode(char* name, struct TypeTable * type,bool isPtr, int size1, int flabel);
void insertParamListToGST(struct Gsymbol * head, char * name, struct ParamStruct * paramList);

struct ParamStruct * insertToParamList(struct ParamStruct * head,char * name, struct TypeTable * type,bool isPtr );
struct ParamStruct * createParamStructNode(char * name, struct TypeTable * type,bool isPtr);
struct ParamStruct * findinParamList(struct ParamStruct * head, char * name);
int countParameterList(struct ParamStruct * paramlist);

void printSymbolTable(struct Gsymbol * head);
void printLocalSymbolTable(struct Lsymbol * head);
void printParamList(struct ParamStruct * head) ;
