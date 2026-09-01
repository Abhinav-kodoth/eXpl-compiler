#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_BASE 4096

struct Gsymbol {
    char* name;       // name of the variable
    int type;         // type of the variable
    int size1;        // size of the type of the variable
    int size2;        //
    int binding;      // stores the static memory address allocated to the variable
    struct Gsymbol *next;
};


struct Gsymbol* createSymbolNode(char* name, int type, int size1, int size2);

// Returns a pointer to the symbol table entry for the variable, returns NULL otherwise.
struct Gsymbol *find(struct Gsymbol * head ,char * name); 

//Creates a symbol table entry.
struct Gsymbol * insertTable(struct Gsymbol * head , char *name, int type, int size1, int size2); 

void printSymbolTable(struct Gsymbol * head);

