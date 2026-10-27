#ifndef LIBFUNCS_H
#define LIBFUNCS_H
#include <stdio.h>

void write(int reg, FILE* targetFile);
void read(int addr, FILE* targetFile);
int heapset(FILE* targetFile);
int alloc(FILE* targetFile);
int free_(int reg, FILE* targetFile);
#endif