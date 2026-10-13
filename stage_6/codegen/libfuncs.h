#ifndef LIBFUNCS_H
#define LIBFUNCS_H
#include <stdio.h>

void write(int reg, FILE* targetFile);
void read(int addr, FILE* targetFile);
void heapset(FILE* targetFile);
int alloc(FILE* targetFile);
void free_(int reg, FILE* targetFile);
#endif