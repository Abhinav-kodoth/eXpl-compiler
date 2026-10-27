#ifndef TREEUTILS_H
#define TREEUTILS_H

typedef struct node node; // Forward declaration of node from tree.h
typedef struct gst gst; // Forward declaration of gst from gst.h
typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct classTable classTable; // Forward declaration of classTable from ct.h

void enterParamsList(node* root, gst* gstEntry);
void assignType(node* root, typeTable* type, classTable* cType);
void installType(node* root);
void installClass(node* root);

#endif