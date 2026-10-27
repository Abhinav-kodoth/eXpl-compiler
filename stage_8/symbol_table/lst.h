#ifndef LST_H
#define LST_H

typedef struct typeTable typeTable; // Forward declaration of typeTable from tt.h
typedef struct classTable classTable; // Forward declaration of classTable from ct.h
typedef struct gst gst; // Forward declaration of gst from gst.h
struct paramStruct;

gst* lstLookup(char* name);
gst* lstInstall(char* name, typeTable* type, classTable* cType, int ptr_level);
gst* globalLookup(char* name);
void freeLst();
void bindParams();

#endif