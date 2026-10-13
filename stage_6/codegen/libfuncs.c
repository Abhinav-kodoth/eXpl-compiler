#include <stdio.h>
#include <stdlib.h>
#include "libfuncs.h"
#include "utils.h"

void write(int reg, FILE* targetFile){
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Write\"");
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "MOV R%d, %d\n", tmp, -2);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", reg);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
}

void read(int addrReg, FILE* targetFile){
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Read\"");
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "MOV R%d, %d\n", tmp, -1);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", addrReg);
    fprintf(targetFile, "PUSH R%d\n", tmp); // 3rd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // ret addr
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
}

void heapset(FILE* targetFile) {
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Heapset\"");
    fprintf(targetFile, "PUSH R%d\n", tmp); // fn name
    fprintf(targetFile, "PUSH R%d\n", tmp); // 1st arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 2nd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 3rd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // ret val
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
}

int alloc(FILE* targetFile) {
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Alloc\"");
    fprintf(targetFile, "PUSH R%d\n", tmp); // fn name
    fprintf(targetFile, "PUSH R%d\n", tmp); // 1st arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 2nd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 3rd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // ret val
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    int retReg = getReg();
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", retReg);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
    return retReg;
}

void free_(int reg, FILE* targetFile) {
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Free\"");
    fprintf(targetFile, "PUSH R%d\n", tmp); // fn name
    fprintf(targetFile, "PUSH R%d\n", reg); // 1st arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 2nd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // 3rd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // ret val
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
}