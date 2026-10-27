#ifndef UTILS_H
#define UTILS_H

typedef struct typeHandle typeHandle; 
typedef struct node node; // Forward declaration of node from tree.h
typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct classTable classTable; // Forward declaration of classTable from ct.h
typedef struct gst gst;
typedef struct paramStruct paramStruct; // Forward declaration of paramStruct

struct typeHandle {
    typeTable* type;
    classTable* cType;
};


int getDeclaredPtrLevel(node* root);
struct typeHandle* getType(node* root);
int checkTypeEquivalence(struct typeHandle *tHandle, typeTable *type, classTable *cType);
void buildLST(node* root, struct paramStruct* paramList);

#endif