#include "TypeTable.h"
#include <stdio.h>
#include <string.h>

struct TypeTable *T_INT, *T_STR, *T_BOOL, *T_VOID;
struct TypeTable *T_INT_PTR, *T_STR_PTR;
struct TypeTable *TUPLE_TYPE, * TUPLE_PTR_TYPE;

struct TypeTable * createTypeTableNode(char * name, int size, struct Fieldlist *fields ) {
    struct TypeTable * node = malloc(sizeof(struct TypeTable));

    if (!node) {
        printf("Fatal Error: Out of memory\n");
        exit(1);
    }

    node->name = strdup(name) ;
    node->size = size ;
    node->fields = fields ;
    node->next = NULL ;
    return node ;
}

struct Fieldlist * createFieldlistNode(char * name, struct TypeTable * type, int fieldIndex) {
    struct Fieldlist * node = malloc(sizeof(struct Fieldlist));

    if(!node) {
        printf("Fatal Error: Out of memory\n");
        exit(1);
    }

    node->name = strdup(name) ;
    node->type = type ;
    node->fieldIndex = fieldIndex ;
    node->next = NULL ;

    return node ;
}

struct TypeTable * createTypeTable() {
    struct TypeTable * head = NULL ;

    //Installing Base Types 
    head = TInstall(head,"int",1,NULL);
    head = TInstall(head,"str",1,NULL);
    head = TInstall(head,"bool",1,NULL);
    head = TInstall(head,"void",0,NULL);
    head = TInstall(head,"int_ptr",1,NULL);
    head = TInstall(head,"str_ptr",1,NULL);

    T_INT       = TLookup(head,"int");
    T_BOOL      = TLookup(head,"bool");
    T_STR       = TLookup(head,"str");
    T_VOID      = TLookup(head,"void");
    T_INT_PTR   = TLookup(head,"int_ptr");
    T_STR_PTR   = TLookup(head,"str_ptr");

    if (!T_INT || !T_STR || !T_BOOL || !T_VOID || !T_INT_PTR || !T_STR_PTR) {
        printf("Fatal Error: Base type initialization failed.\n");
        exit(1);
    }

    return head ;
}

struct TypeTable* TLookup(struct TypeTable * head ,char *name)  {
    struct TypeTable *temp = head;
    while (temp) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL; 
}

struct TypeTable* TInstall(struct TypeTable * head,char *name,int size, struct Fieldlist *fields) {
    if(TLookup(head,name) != NULL) {
        printf("Redeclaration of tuple %s\n",name);
        exit(1);
    }

    struct TypeTable * node = createTypeTableNode(name,size,fields);

    if(head == NULL) {
        return node ;
    }

    struct TypeTable * temp = head ;
    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;

    return head ;
}

struct Fieldlist* FLookup(struct TypeTable *type, char *name) {
    if(type == NULL) {
        return NULL ;
    }

    struct Fieldlist * field = type->fields ;
    while(field) {
        if(strcmp(field->name,name) == 0) {
            return field ;
        }
        field = field->next ;
    }
    return NULL ;
}

struct Fieldlist* FInstall(struct Fieldlist* fieldHead,char * name, struct TypeTable * type,int fieldIndex) {
    struct Fieldlist * node = createFieldlistNode(name,type,fieldIndex);

    if(fieldHead == NULL) {
        return node ;
    }
    struct Fieldlist * temp = fieldHead ;
    while(temp->next) {
        temp = temp->next ;
    }

    temp->next = node ;
    return fieldHead ;
}

int GetSize(struct TypeTable * type) {
    if(type) {
        return type->size ;
    }
    return -1 ;
}