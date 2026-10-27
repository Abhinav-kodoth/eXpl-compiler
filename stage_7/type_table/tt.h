#ifndef TT_H
#define TT_H

struct fieldList {
    char *name;
    int fieldIndex;
    struct typeTable *type;
    struct fieldList *next;
};

typedef struct typeTable {
    char *name;
    int size;
    struct fieldList *fields;
    struct typeTable *next;
} typeTable;

typedef struct fieldList fieldList;

void ttInitialize();
typeTable* ttLookup(char *name);
typeTable* ttInstall(char *name);
void ttAddField(char* typeName, char* name); // Installs the field in last entry of TT
fieldList* ttFieldLookup(char* type, char* fieldName);
void ttPrint();

#endif