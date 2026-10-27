#ifndef CT_H
#define CT_H

struct paramStruct;

struct classTable {
    char *name;
    struct cFieldList *memberFields;
    struct cMethodList *memberMethods;
    struct classTable *parentClass;
    int classIndex;
    int fieldCount;
    int methodCount;
    struct classTable *next;
};

struct cFieldList {
    char *name;
    int fieldIndex;
    struct typeTable *type;
    struct classTable *cType;
    struct cFieldList *next;
};

struct cMethodList {
    char *name;
    struct typeTable *type;
    struct paramStruct *params;
    int funcIndex;
    int funcLabel;
    struct cMethodList *next;
};

typedef struct classTable classTable;
typedef struct cFieldList cFieldList;
typedef struct cMethodList cMethodList;
typedef struct paramStruct paramStruct;

classTable *ctLookup(char *name);
void ctInstall(char *name);
void ctAddField(char* name, char* classOrTypeName); // Installs the field in last entry of CT
void ctAddMethod(char* name, char* returnType);     // Installs the method in last entry of CT
void ctAddMethodParam(char* name, char* typeName);  // Adds param to the last added method
cFieldList* ctFieldLookup(char* className, char* fieldName);
cMethodList* ctMethodLookup(char* className, char* methodName);
classTable* getLastClass();
void ctPrint();


#endif