#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../type_table/tt.h"
#include "utils.h"
#include "libfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int regCount = 0;
int label = 0;
int fnlabel = 0;

labelStack* labelStackTop = NULL;
extern gst* gstRoot;
extern gst* lstRoot;

extern int binaryOpHandler(node* root, FILE* targetFile);

int getAddr(node* varNode, FILE* targetFile);
int codeGen(node* root, FILE* targetFile);

int getAddr(node* varNode, FILE* targetFile) {
    gst* entry = globalLookup(gstRoot, lstRoot, varNode->varname);
    if(entry == NULL) {
        printf("Codegen Error: Undeclared variable '%s'\n", varNode->varname);
        exit(1);
    }
    varNode->type = entry->type; 
    if(entry->relativeBinding != -1) {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, BP\n", reg);
        fprintf(targetFile, "ADD R%d, %d\n", reg, entry->relativeBinding);
        return reg;
    }
    int reg = getReg();
    fprintf(targetFile, "MOV R%d, %d\n", reg, entry->binding);
    return reg;
}
int getArrayAddr(FILE* targetFile, node* arrayNode) {
    int baseAddr = getAddr(arrayNode, targetFile);
    int indexReg = codeGen(arrayNode->right, targetFile);
    fprintf(targetFile, "ADD R%d, R%d\n", baseAddr, indexReg);
    freeReg(); // Free indexReg
    return baseAddr;
}
int getFieldAddr(FILE* targetFile, node* fieldNode) {
    if(fieldNode->nodetype != NODE_FIELD) {
        return getAddr(fieldNode, targetFile);
    }

    int reg = getFieldAddr(targetFile, fieldNode->left);
    typeTable* leftType = getType(fieldNode->left);
    fieldList* field = ttFieldLookup(leftType->name, fieldNode->right->varname);
    fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
    fprintf(targetFile, "ADD R%d, %d\n", reg, field->fieldIndex);
    return reg;
}

int codeGen(node* root, FILE* targetFile) {
    if(!root) {
        return -1;
    }

    switch(root->nodetype) {
        case NODE_NUM: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
            return reg;
        }
        case NODE_STR: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %s\n", reg, root->strval);
            return reg;
        }
        case NODE_ID: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, getAddr(root, targetFile));
            freeReg(); // Free address register
            return reg;
        }
        case NODE_ARRAY: {
            int reg = getArrayAddr(targetFile, root);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        case NODE_PLUS:
        case NODE_MINUS:
        case NODE_MUL:
        case NODE_DIV: 
        case NODE_MOD:
        case NODE_GT:
        case NODE_GTE:
        case NODE_LT:
        case NODE_LTE:
        case NODE_EQ:
        case NODE_NEQ:
        {
            return binaryOpHandler(root, targetFile);
        }
        case NODE_ASSIGN: {
            int rightReg = codeGen(root->right, targetFile);
            if(root->left->nodetype == NODE_ARRAY) {
                int leftReg = getArrayAddr(targetFile, root->left);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, rightReg);
                freeReg(); 
            }
            else if(root->left->nodetype == NODE_PTR) {
                int leftReg = codeGen(root->left->right, targetFile);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, rightReg);
                freeReg();
            }
            else if(root->left->nodetype == NODE_FIELD) {
                int leftReg = getFieldAddr(targetFile, root->left);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, rightReg);
                freeReg();
            }
            else {
                int leftAddr = getAddr(root->left, targetFile);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftAddr, rightReg);
                freeReg();
            }
            freeReg();
            return -1;
        }
        case NODE_WRITE: {
            int leftReg = codeGen(root->left, targetFile);
            write(leftReg, targetFile);
            freeReg();
            return -1;
        }
        case NODE_READ: {
            if(root->left->nodetype == NODE_ARRAY) {
                int arrayReg = getArrayAddr(targetFile, root->left);
                read(arrayReg, targetFile);
                freeReg(); // Free arrayReg
            }
            else if(root->left->nodetype == NODE_PTR) {
                int reg = codeGen(root->left->right, targetFile);
                read(reg, targetFile);
                freeReg();
            }
            else if(root->left->nodetype == NODE_FIELD) {
                int fieldAddr = getFieldAddr(targetFile, root->left);
                read(fieldAddr, targetFile);
                freeReg();
            }
            else {
                int leftAddr = getAddr(root->left, targetFile);
                read(leftAddr, targetFile);
                freeReg();
            }
            return -1;
        }
        case NODE_CONNECTOR: {
            codeGen(root->left, targetFile);
            codeGen(root->right, targetFile);
            return -1;
        }
        case NODE_IFELSE: {
            int falseLabel = getLabel();
            int endLabel = getLabel();
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, falseLabel);
            codeGen(root->right->left, targetFile);
            fprintf(targetFile, "JMP L%d\n", endLabel);
            fprintf(targetFile, "L%d:\n", falseLabel);
            codeGen(root->right->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            return -1;
        }
        case NODE_IF: {
            int endLabel = getLabel();
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            return -1;
        }
        case NODE_WHILE: {
            int condLabel = getLabel();
            int endLabel  = getLabel();
            labelStackTop = pushLabelStack(labelStackTop, condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", condLabel);
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "JMP L%d\n", condLabel);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();

            labelStackTop = popLabelStack(labelStackTop);
            return -1;
        }
        case NODE_BREAK: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->end);
            return -1;
        }
        case NODE_CONTINUE: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->cond);
            return -1;
        }
        case NODE_BRKP: {
            fprintf(targetFile, "BRKP\n");
            return -1;
        }
        case NODE_DOWHILE: {
            int stmtLabel = getLabel();
            int condLabel = getLabel();
            int endLabel  = getLabel();
            labelStackTop = pushLabelStack(labelStackTop, condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", stmtLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", condLabel);
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JNZ R%d, L%d\n", condReg, stmtLabel);
            fprintf(targetFile, "L%d:\n", endLabel);

            labelStackTop = popLabelStack(labelStackTop);
            freeReg();
            return -1;
        }
        case NODE_DECL: {
            return -1;
        }
        case NODE_TYPE: {
            return -1;
        }
        case NODE_PTR: {
            int reg = codeGen(root->right, targetFile);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        case NODE_ADDR_OF: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, R%d\n", reg, getAddr(root->right, targetFile));
            return reg;
        }
        case NODE_FNDEF: {
            if(root->gstEntry->fLabel != -1) {
                printf("Error: Redeclaration of Function '%s'.\n", root->varname);
                exit(1);
            }
            int fnLabel = getFnLabel();
            fprintf(targetFile, "F%d:\n", fnLabel);
            root->gstEntry->fLabel = fnLabel;

            fprintf(targetFile, "PUSH BP\n");  // Save caller's BP
            fprintf(targetFile, "MOV BP, SP\n");  // Set BP to current SP

            struct paramStruct* params = root->gstEntry->paramList;
            while(params != NULL) {
                lstRoot = lstInstall(lstRoot, params->name, params->type, params->ptr_level);
                params = params->next;
            }
            bindParams(lstRoot); // assign bindings to parameters
            buildLST(root->right->left, &lstRoot, root->gstEntry->paramList);

            gst* localHead = lstRoot;
            gst* localCursor = localHead;
            
            int localVarReserve = 0;
            while(localCursor != NULL) {
                if(localCursor->binding != -1 || localCursor->relativeBinding >= 0) {
                    localVarReserve++;
                }
                localCursor = localCursor->next;
            }
            if(localVarReserve > 0) {
                fprintf(targetFile, "ADD SP, %d\n", localVarReserve); // Reserve space for local variables
            }

            codeGen(root->right, targetFile);

            fprintf(targetFile, "MOV SP, BP\n"); // Deallocate local variables
            fprintf(targetFile, "POP BP\n"); // Restore caller's BP
            fprintf(targetFile, "RET\n");

                freeLst(localHead);
                lstRoot = NULL;
            return -1;
        }
        case NODE_LDECL: {
            return -1;
        }
        case NODE_RETURN: {
            int retReg = codeGen(root->left, targetFile);
            int retAddrReg = getReg();
            fprintf(targetFile, "MOV R%d, BP\n", retAddrReg);
            fprintf(targetFile, "SUB R%d, 2\n", retAddrReg);
            fprintf(targetFile, "MOV [R%d], R%d\n", retAddrReg, retReg);
            freeReg();
            freeReg();
            return -1;
        }
        case NODE_MAIN: {
            fprintf(targetFile, "M0:\n");
            fprintf(targetFile, "MOV BP, SP\n");
            buildLST(root->left, &lstRoot, NULL);

            //Reserve space for local vars
            gst* localCursor = lstRoot;
            while(localCursor != NULL) {
                if(localCursor->binding != -1 || localCursor->relativeBinding >= 0) {
                    fprintf(targetFile, "PUSH R0\n"); // Allocate space for local variable
                }
                localCursor = localCursor->next;
            }

            codeGen(root->right, targetFile);
            fprintf(targetFile, "INT 10\n");
            freeLst(lstRoot);
            return -1;
        }
        case NODE_FNCALL: {
            int regCount = getRegCount();
            printf("Pushed %d registers before function call\n", regCount);
            for(int i = 0; i < regCount; i++) {
                fprintf(targetFile, "PUSH R%d\n", i);
            }
            
            pushRegStack();
            codeGen(root->right, targetFile); // evaluate args
            fprintf(targetFile, "PUSH R0\n"); // space for return value
            fprintf(targetFile, "CALL F%d\n", root->gstEntry->fLabel); // Transfer control

            popRegStack();

            int retReg = getReg();
            fprintf(targetFile, "POP R%d\n", retReg); // Get return value

            // Clean up arguments from stack
            int argCount = 0;
            struct paramStruct* paramList = root->gstEntry->paramList;
            while(paramList) {
                argCount++;
                paramList = paramList->next;
            }
            if(argCount > 0) {
                fprintf(targetFile, "SUB SP, %d\n", argCount);
            }

            for(int i = regCount - 1; i >= 0; i--) {
                fprintf(targetFile, "POP R%d\n", i);
            }

            return retReg;
        }
        case NODE_ARG: {
            int argReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "PUSH R%d\n", argReg);
            freeReg();
            return -1;
        }
        case NODE_TYPEDEF: {
            return -1;
        }
        case NODE_FIELDDECL: {
            return -1;
        }
        case NODE_FIELD: {
            int reg = getFieldAddr(targetFile, root);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        case NODE_INITIALIZE: {
            heapset(targetFile);
            return -1;
        }
        case NODE_ALLOC: {
            int addrReg = alloc(targetFile);
            return addrReg;
        }
        case NODE_FREE: {
            int reg = codeGen(root->left, targetFile);
            free_(reg, targetFile);
            freeReg();
            return -1;
        }
        case NODE_NULL: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, 0\n", reg);
            return reg;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}

void printAST(node* root, const char* prefix, int isLast) {
    if (!root) return;
    
    printf("%s", prefix);
    printf("%s", isLast ? "└── " : "├── ");
    
    switch(root->nodetype) {
        case NODE_NUM:
            printf("NUM(%d)\n", root->val);
            break;
        case NODE_ID:
            printf("ID(%s)\n", root->varname);
            break;
        case NODE_PLUS:
            printf("PLUS\n");
            break;
        case NODE_MINUS:
            printf("MINUS\n");
            break;
        case NODE_MUL:
            printf("MUL\n");
            break;
        case NODE_DIV:
            printf("DIV\n");
            break;
        case NODE_GT:
            printf("GT\n");
            break;
        case NODE_GTE:
            printf("GTE\n");
            break;
        case NODE_LT:
            printf("LT\n");
            break;
        case NODE_LTE:
            printf("LTE\n");
            break;
        case NODE_EQ:
            printf("EQ\n");
            break;
        case NODE_NEQ:
            printf("NEQ\n");
            break;
        case NODE_ASSIGN:
            printf("ASSIGN\n");
            break;
        case NODE_CONNECTOR:
            printf("CONNECTOR\n");
            break;
        case NODE_IF:
            printf("IF\n");
            break;
        case NODE_IFELSE:
            printf("IFELSE\n");
            break;
        case NODE_WHILE:
            printf("WHILE\n");
            break;
        case NODE_DOWHILE:
            printf("DOWHILE\n");
            break;
        case NODE_BREAK:
            printf("BREAK\n");
            break;
        case NODE_CONTINUE:
            printf("CONTINUE\n");
            break;
        case NODE_READ:
            printf("READ\n");
            break;
        case NODE_WRITE:
            printf("WRITE\n");
            break;
        case NODE_BRKP:
            printf("BRKP\n");
            break;
        case NODE_DECL:
            printf("DECL\n");
            break;
        case NODE_TYPE:
            printf("TYPE (%s)\n", root->type->name);
            break;
        case NODE_PTR:
            printf("PTR(%s)\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_ADDR_OF:
            printf("ADDR_OF(%s)\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_ARRAY:
            printf("ARRAY(%s)\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_FNDECL:
            printf("FNDECL - %s()\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_FNDEF:
            printf("FNDEF - %s()\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_FNCALL:
            printf("FNCALL - %s()\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_PARAM:
            printf("PARAM(%s)\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_RETURN:
            printf("RETURN\n");
            break;
        case NODE_STR:
            printf("STR(%s)\n", root->strval);
            break;
        case NODE_MAIN:
            printf("MAIN\n");
            break;
        case NODE_ARG:
            printf("ARG\n");
            break;
        case NODE_LDECL:
            printf("LDECL\n");
            break;
        case NODE_FIELDDECL:
            printf("FIELDDECL\n");
            break;
        case NODE_FIELD:
            printf("FIELD\n");
            break;
        default:
            printf("UNKNOWN(%d)\n", root->nodetype);
    }
    
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "│   ");
    
    if (root->left) printAST(root->left, newPrefix, !root->right);
    if (root->right) printAST(root->right, newPrefix, 1);
}