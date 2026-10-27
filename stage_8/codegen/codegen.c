#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../type_table/tt.h"
#include "../class_table/ct.h"
#include "libfuncs.h"
#include "../utils/utils.h"
#include "../utils/codegenUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int regCount;
extern int label;
extern int fnlabel;

extern labelStack* labelStackTop;
extern gst* gstRoot;
extern gst* lstRoot;
extern classTable* currClass;

int getAddr(node* varNode, FILE* targetFile);
pair* codeGen(node* root, FILE* targetFile);

int getAddr(node* varNode, FILE* targetFile) {
    gst* entry = globalLookup(varNode->varname);
    if(entry == NULL) {
        printf("Codegen Error: Undeclared variable '%s'\n", varNode->varname);
        exit(1);
    }
    varNode->type = entry->type; 
    varNode->cType = entry->cType;
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
    pair* indexRet = codeGen(arrayNode->right, targetFile);
    int indexReg = indexRet->r1;
    fprintf(targetFile, "ADD R%d, R%d\n", baseAddr, indexReg);
    freeReg(); // Free indexReg
    free(indexRet);
    return baseAddr;
}
int getFieldAddr(FILE* targetFile, node* fieldNode) {
    if(fieldNode->nodetype != NODE_FIELD) {
        return getAddr(fieldNode, targetFile);
    }

    int reg = getFieldAddr(targetFile, fieldNode->left);
    typeHandle* leftType = getType(fieldNode->left);
    int fieldIndex;
    if(leftType->type) {
        fieldIndex = ttFieldLookup(leftType->type->name, fieldNode->right->varname)->fieldIndex;
    }
    else {
        fieldIndex = ctFieldLookup(leftType->cType->name, fieldNode->right->varname)->fieldIndex;
    }
    fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
    fprintf(targetFile, "ADD R%d, %d\n", reg, fieldIndex);
    return reg;
}

pair* codeGen(node* root, FILE* targetFile) {
    if(!root) {
        return NULL;
    }

    switch(root->nodetype) {
        case NODE_NUM: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
            return createPair(reg, -1);
        }
        case NODE_STR: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %s\n", reg, root->strval);
            return createPair(reg, -1);
        }
        case NODE_ID: {
            int reg = getReg();
            int reg2 = getReg();
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, getAddr(root, targetFile));
            if(getType(root)->cType != NULL) {
                fprintf(targetFile, "MOV R%d, R%d\n", reg2, getAddr(root, targetFile)); // Load object address
                fprintf(targetFile, "INR R%d\n", reg2); // Move to vft address
                fprintf(targetFile, "MOV R%d, [R%d]\n", reg2, reg2); // Load vft address
            }
            else{
                freeReg(); // Free reg2 as it's not used
            }
            freeReg(); // Free address register
            return createPair(reg, reg2);
        }
        case NODE_ARRAY: {
            int reg = getArrayAddr(targetFile, root);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return createPair(reg, -1);
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
            pair* ret = codeGen(root->right, targetFile);
            int rightReg = ret->r1;
            if(getType(root->left)->cType != NULL) {
                int leftReg = getAddr(root->left, targetFile);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, ret->r1); // object address
                fprintf(targetFile, "INR R%d\n", leftReg);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, ret->r2); // vft address
                freeReg(); // Free leftReg
            }
            else if(root->left->nodetype == NODE_ARRAY) {
                int leftReg = getArrayAddr(targetFile, root->left);
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, rightReg);
                freeReg(); 
            }
            else if(root->left->nodetype == NODE_PTR) {
                pair* ret = codeGen(root->left->right, targetFile);
                int leftReg = ret->r1;
                fprintf(targetFile, "MOV [R%d], R%d\n", leftReg, rightReg);
                freeReg();
                free(ret);
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
            
            if(ret->r1 != -1) freeReg(); // Free rightReg if it was used
            if(ret->r2 != -1) freeReg(); // Free rightReg if it was used

            free(ret);
            return NULL;
        }
        case NODE_WRITE: {
            pair* ret = codeGen(root->left, targetFile);
            int leftReg = ret->r1;
            write(leftReg, targetFile);
            freeReg();
            free(ret);
            return NULL;
        }
        case NODE_READ: {
            if(root->left->nodetype == NODE_ARRAY) {
                int arrayReg = getArrayAddr(targetFile, root->left);
                read(arrayReg, targetFile);
                freeReg(); // Free arrayReg
            }
            else if(root->left->nodetype == NODE_PTR) {
                pair* ret = codeGen(root->left->right, targetFile);
                int reg = ret->r1;
                read(reg, targetFile);
                freeReg();
                free(ret);
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
            return NULL;
        }
        case NODE_CONNECTOR: {
            pair* left = codeGen(root->left, targetFile);
            pair* right = codeGen(root->right, targetFile);
            
            if(left != NULL) freeReg();
            if(right != NULL) freeReg();

            free(left);
            free(right);
            return NULL;
        }
        case NODE_IFELSE: {
            int falseLabel = getLabel();
            int endLabel = getLabel();
            pair* condRet = codeGen(root->left, targetFile);
            int condReg = condRet->r1;
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, falseLabel);
            codeGen(root->right->left, targetFile);
            fprintf(targetFile, "JMP L%d\n", endLabel);
            fprintf(targetFile, "L%d:\n", falseLabel);
            codeGen(root->right->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            free(condRet);
            return NULL;
        }
        case NODE_IF: {
            int endLabel = getLabel();
            pair* condRet = codeGen(root->left, targetFile);
            int condReg = condRet->r1;
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            free(condRet);
            return NULL;
        }
        case NODE_WHILE: {
            int condLabel = getLabel();
            int endLabel  = getLabel();
            pushLabelStack(condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", condLabel);
            pair* condRet = codeGen(root->left, targetFile);
            int condReg = condRet->r1;
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "JMP L%d\n", condLabel);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();

            labelStackTop = popLabelStack();
            free(condRet);
            return NULL;
        }
        case NODE_BREAK: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->end);
            return NULL;
        }
        case NODE_CONTINUE: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->cond);
            return NULL;
        }
        case NODE_BRKP: {
            fprintf(targetFile, "BRKP\n");
            return NULL;
        }
        case NODE_DOWHILE: {
            int stmtLabel = getLabel();
            int condLabel = getLabel();
            int endLabel  = getLabel();
            pushLabelStack(condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", stmtLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", condLabel);
            pair* condRet = codeGen(root->left, targetFile);
            int condReg = condRet->r1;
            fprintf(targetFile, "JNZ R%d, L%d\n", condReg, stmtLabel);
            fprintf(targetFile, "L%d:\n", endLabel);

            labelStackTop = popLabelStack();
            freeReg();
            free(condRet);
            return NULL;
        }
        case NODE_DECL: {
            return NULL;
        }
        case NODE_TYPE: {
            return NULL;
        }
        case NODE_PTR: {
            pair* ret = codeGen(root->right, targetFile);
            int reg = ret->r1;
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            free(ret);
            return createPair(reg, -1);
        }
        case NODE_ADDR_OF: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, R%d\n", reg, getAddr(root->right, targetFile));
            return createPair(reg, -1);
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
                lstInstall(params->name, params->type, params->cType, params->ptr_level);
                params = params->next;
            }
            bindParams(); // assign bindings to parameters
            buildLST(root->right->left, root->gstEntry->paramList);

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

            freeLst();
            return NULL;
        }
        case NODE_LDECL: {
            return NULL;
        }
        case NODE_RETURN: {
            pair* ret = codeGen(root->left, targetFile);
            int retReg = ret->r1;
            int retAddrReg = getReg();
            fprintf(targetFile, "MOV R%d, BP\n", retAddrReg);
            fprintf(targetFile, "SUB R%d, 2\n", retAddrReg);
            fprintf(targetFile, "MOV [R%d], R%d\n", retAddrReg, retReg);
            freeReg();
            freeReg();
            return NULL;
        }
        case NODE_MAIN: {
            fprintf(targetFile, "M0:\n");
            fprintf(targetFile, "MOV BP, SP\n");
            buildLST(root->left, NULL);

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
            freeLst();
            return NULL;
        }
        case NODE_FNCALL: {
            int regCount = getRegCount();
            
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

            return createPair(retReg, -1);
        }
        case NODE_ARG: {
            pair* ret = codeGen(root->left, targetFile);
            int argReg = ret->r1;
            fprintf(targetFile, "PUSH R%d\n", argReg);
            freeReg();
            return createPair(-1, -1);
        }
        case NODE_TYPEDEF: {
            return NULL;
        }
        case NODE_FIELDDECL: {
            return NULL;
        }
        case NODE_FIELD: {
            int reg = getFieldAddr(targetFile, root);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return createPair(reg, -1);
        }
        case NODE_INITIALIZE: {
            int reg = heapset(targetFile);
            return createPair(reg, -1);
        }
        case NODE_NEW: {
            int addrReg = alloc(targetFile);
            int vftReg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", vftReg, root->cType->vft);
            return createPair(addrReg, vftReg);
        }
        case NODE_ALLOC: {
            int addrReg = alloc(targetFile);
            return createPair(addrReg, -1);
        }
        case NODE_FREE: {
            pair* retn = codeGen(root->left, targetFile);
            int reg = retn->r1;
            int ret = free_(reg, targetFile);
            fprintf(targetFile, "MOV R%d, R%d\n", reg, ret); // so that we can free a register
            freeReg();  // this frees ret, not reg
            free(retn);
            return createPair(reg, -1);
        }
        case NODE_NULL: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, 0\n", reg);
            return createPair(reg, -1);
        }
        case NODE_CDEF: {
            currClass = ctLookup(root->varname);
            codeGen(root->right->right, targetFile);
            currClass = NULL;
            return NULL;
        }
        case NODE_CFNDEF: {
            cMethodList* methodEntry = ctMethodLookup(currClass->name, root->varname);

            int fnLabel = methodEntry->funcLabel;
            fprintf(targetFile, "F%d:\n", fnLabel);

            fprintf(targetFile, "PUSH BP\n");  // Save caller's BP
            fprintf(targetFile, "MOV BP, SP\n");  // Set BP to current SP

            struct paramStruct* params = methodEntry->params;

            // Build LST (Includes semantic checks for redeclaration of local variables and parameters)
            params = methodEntry->params; // reset params pointer to head of list
            lstInstall("self", NULL, ctLookup(currClass->name), 0); // install self parameter
            lstInstall("__self_vft", NULL, NULL, 0); // install hidden parameter for vft pointer
            while(params != NULL) {
                lstInstall(params->name, params->type, params->cType, params->ptr_level);
                params = params->next;
            }
            bindParams(); // assign bindings to parameters
            
            buildLST(root->right->left, methodEntry->params);

            // printGST(lstRoot);

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

            codeGen(root->right->right, targetFile);

            fprintf(targetFile, "MOV SP, BP\n"); // Deallocate local variables
            fprintf(targetFile, "POP BP\n"); // Restore caller's BP
            fprintf(targetFile, "RET\n");

            freeLst();
            return NULL;
        }
        case NODE_FIELDFN: {
            int regCount = getRegCount();

            for(int i = 0; i < regCount; i++) {
                fprintf(targetFile, "PUSH R%d\n", i);
            }
            
            pushRegStack();

            typeHandle* type = getType(root->left);
            cMethodList* methodEntry = ctMethodLookup(type->cType->name, root->right->left->varname);
            
            // Push self pointer
            int selfReg = getFieldAddr(targetFile, root->left);  // Addr in stack, that holds addr to heap
            fprintf(targetFile, "MOV R%d, [R%d]\n", selfReg, selfReg); // Addr in heap
            fprintf(targetFile, "PUSH R%d\n", selfReg);  // Push self pointer value
            // Push vft pointer
            selfReg = getFieldAddr(targetFile, root->left);  // Addr in stack, that holds addr to heap
            fprintf(targetFile, "INR R%d\n", selfReg); // Move to vft (word next to object address)
            fprintf(targetFile, "MOV R%d, [R%d]\n", selfReg, selfReg); // Load vft
            fprintf(targetFile, "PUSH R%d\n", selfReg);  // Push vft pointer value
            freeReg();
            codeGen(root->right->right, targetFile); // evaluate args
            fprintf(targetFile, "PUSH R0\n"); // space for return value

            int objectAddressReg = getAddr(root->left, targetFile);
            fprintf(targetFile, "INR R%d\n", objectAddressReg); // Move to vft (word next to object address)
            fprintf(targetFile, "MOV R%d, [R%d]\n", objectAddressReg, objectAddressReg); // Load vft
            fprintf(targetFile, "ADD R%d, %d\n", objectAddressReg, methodEntry->funcIndex); // Move to method address in vft
            fprintf(targetFile, "MOV R%d, [R%d]\n", objectAddressReg, objectAddressReg); // Load method address
            fprintf(targetFile, "CALL R%d\n", objectAddressReg); // Transfer control

            freeReg(); // Free objectAddressReg
            popRegStack();

            int retReg = getReg();
            fprintf(targetFile, "POP R%d\n", retReg); // Get return value

            // Clean up arguments from stack
            int argCount = 1;  // start from 1 cuz self is not in param list
            struct paramStruct* paramList = methodEntry->params;
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

            return createPair(retReg, -1);
        }
        case NODE_EXTENDS: {
            return codeGen(root->right, targetFile);
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
        case NODE_TYPEDEF:
            printf("TYPEDEF - %s\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_CDEF:
            printf("CLASS DEF - %s\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_CFIELD:
            printf("CLASS FIELD\n");
            break;
        case NODE_CMETHOD:
            printf("CLASS METHOD - %s\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_CFNDEF:
            printf("CLASS FNDEF - %s\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_NULL:
            printf("NULL\n");
            break;
        case NODE_EXTENDS:
            printf("EXTENDS\n");
            break;
        default:
            printf("UNKNOWN(%d)\n", root->nodetype);
    }
    
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "│   ");
    
    if (root->left) printAST(root->left, newPrefix, !root->right);
    if (root->right) printAST(root->right, newPrefix, 1);
}
