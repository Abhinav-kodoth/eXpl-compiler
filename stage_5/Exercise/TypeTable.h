#ifndef TYPETABLE_H  
#define TYPETABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct TypeTable{
    char *name;                 //type name
    int size;                   //size of the type
    struct Fieldlist *fields;   //pointer to the head of fields list
    struct TypeTable *next;     // pointer to the next type table entry
};

struct Fieldlist{
    char *name;              //name of the field
    int fieldIndex;          //the position of the field in the field list
    struct TypeTable *type;  //pointer to type table entry of the field's type
    struct Fieldlist *next;  //pointer to the next field
};

extern struct TypeTable *T_INT, *T_STR, *T_BOOL, *T_VOID;
extern struct TypeTable *T_INT_PTR, *T_STR_PTR;
extern struct TypeTable *TUPLE_TYPE, * TUPLE_PTR_TYPE;

struct TypeTable * createTypeTableNode(char * name, int size, struct Fieldlist *fields );

struct Fieldlist * createFieldlistNode(char * name, struct TypeTable * type, int fieldIndex);

struct Fieldlist* FInstall(struct Fieldlist* fieldHead, char * name, struct TypeTable * type, int fieldIndex);

struct TypeTable* createTypeTable();

struct TypeTable* TLookup(struct TypeTable *head,char *name) ;

struct TypeTable* TInstall(struct TypeTable *head,char *name,int size, struct Fieldlist *fields);

struct Fieldlist* FLookup(struct TypeTable *type, char *name);

int GetSize(struct TypeTable * type);

#endif