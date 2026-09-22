#include "codeGen.h"
#include "exptree.h"
#include "symbolTable.h"

#define STACK_SIZE 100

int continueStack[STACK_SIZE] ;
int breakStack[STACK_SIZE];

int stackTop = -1 ;
int maxLabel  = -1;
int maxReg = -1;

int getLabel() {
    return ++maxLabel ;
}

int getReg() {
    if(maxReg < 20) {
        //printf("getReg -> %d\n",maxReg);
       return ++maxReg ;
    } else {
        printf("Out of registers\n");
        exit(1);
    }
    return -1 ;
} 

void freeReg() {
    if(maxReg >= 0) {
        //printf("freeReg -> %d\n",maxReg);
        maxReg-- ;
    } else {
        printf("Error freeing registers\n");
        exit(1);
    }
}

int getBreakLabel() {
    return breakStack[stackTop];
}

int getContinueLabel() {
    return continueStack[stackTop];
}

void pushLabel(int breakLabel, int continueLabel) {
    if(stackTop + 1 >= STACK_SIZE) {
        printf("Stack Size Exceeded\n");
        exit(1);
    }
    stackTop++ ;

    breakStack[stackTop] = breakLabel ;
    continueStack[stackTop] = continueLabel ;
}

void popLabel() {
    if(stackTop < 0) {
        printf("Popping from Empty Stack\n");
        exit(1) ;
    }
    stackTop-- ;
}

int countLocalVariables(struct Lsymbol * Lhead) {
    struct Lsymbol * temp = Lhead ;

    int count = 0;
    while(temp) {
        if(temp->binding > 0) {
            count++ ;
        }
        temp = temp->next ;
    }
    return count ;
}


int getAddress(struct tnode * root,FILE * op) {
    switch(root->nodetype) {
        case ARR_NODE : { 
            int baseReg = getReg();
            int offsetReg = codeGen(root->left,op);
            int baseAddress = root->Gentry->binding ;

            fprintf(op, "MOV R%d, %d\n",baseReg,baseAddress);
            fprintf(op, "ADD R%d, R%d\n",baseReg,offsetReg );
            freeReg();
            return baseReg ;
        }
        case ID_NODE : {
            int r1 = getReg();
            
            if(root->Lentry != NULL) {
                int offset = root->Lentry->binding ;
                fprintf(op, "MOV R%d, BP\n",r1);
                fprintf(op, "ADD R%d, %d\n",r1,offset);
            } else {
                int offset = root->Gentry->binding ;
                fprintf(op, "MOV R%d, %d\n",r1,offset);
            }
            return r1 ;
        }
        case DEREF_NODE : {
            int ptrAddress = getAddress(root->left,op);

            fprintf(op,"MOV R%d, [R%d]\n",ptrAddress,ptrAddress);

            return ptrAddress ;
        }
    }
}


int evalExpression(struct tnode * root,FILE * op) {
    if (!root) { 
        printf("evalExpression called with NULL\n");
        exit(1); 
    }

    switch(root->nodetype) {
        case ARR_NODE : { 
            int baseReg = getReg();
            int offsetReg = codeGen(root->left,op);
            int baseAddress = root->Gentry->binding ;

            fprintf(op, "MOV R%d, %d\n",baseReg,baseAddress);
            fprintf(op, "ADD R%d, R%d\n",baseReg,offsetReg );
            fprintf(op, "MOV R%d, [R%d]\n",baseReg,baseReg);
            freeReg();
            return baseReg ;
        }

        case ID_NODE : {
            int r1 = getAddress(root,op);
            fprintf(op,"MOV R%d, [R%d]\n",r1,r1);
            return r1 ;
        }
        case DEREF_NODE : {
            int r1 = getReg();
            int address = root->Gentry->binding ;

            fprintf(op, "MOV R%d, %d\n",r1,address);
            fprintf(op, "MOV R%d, [R%d]\n",r1,r1);
            fprintf(op, "MOV R%d, [R%d]\n",r1,r1);
            return r1 ;
        }
        case NUM_NODE : {
            int r1 = getReg();
            fprintf(op, "MOV R%d, %d\n",r1,root->val);
            return r1;
        }
    }
}


void auxFunctions(FILE * op, int codePrint, int reg1, int reg2) {
    switch(codePrint) {
        case ENTRY_CODE : {
            fprintf(op, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",0,2056,0,0,0,0,0,0);
            fprintf(op, "MOV SP, 4200\n");
            break;
        }

        case  EXIT_CODE : {
            fprintf(op, "MOV R0, \"Exit\"\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "CALL 0\n");
            break;
        }

        case  READ_CODE : {
            fprintf(op, "MOV R%d, \"Read\"\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, -1\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "PUSH R%d\n",reg2);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);

            fprintf(op, "CALL 0\n");

            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            break;
        }

        case  WRITE_CODE : {
            fprintf(op, "MOV R%d, \"Write\"\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, -2\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "PUSH R%d\n",reg2);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);

            fprintf(op, "CALL 0\n");

            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            fprintf(op, "POP R%d\n",reg1);
            break;
        }
    }
}

void funcCodeGen(struct Gsymbol * Ghead, FILE * op) {
    struct Gsymbol * func =  Ghead ;

    if(func->flabel != -1) {
        fprintf(op, "F%d:\n", func->flabel);
        fprintf(op, "PUSH BP\n");
        fprintf(op, "MOV BP, SP\n");

        int localVarCount = countLocalVariables(func->Lentry);
        printf("%d\n",localVarCount);

        fprintf(op, "ADD SP, %d\n",localVarCount);

        if(func->bodyAST) codeGen(func->bodyAST,op);

    }
}

int codeGen(struct tnode * root, FILE * op) {
    int r1, r2, label1, label2 ;

    switch(root->nodetype) {
        case NUM_NODE :{
            r1 = getReg();
            fprintf(op, "MOV R%d, %d\n",r1,root->val);
            return r1 ;
        }
        case ID_NODE :{
            return evalExpression(root,op);
        }
        case MATRIX_NODE : {
            return evalExpression(root,op);
        }
        case ARR_NODE : {
            return evalExpression(root,op);
        }
        case STR_NODE :{
            r1 = getReg();
            fprintf(op, "MOV R%d, \"%s\"\n",r1,root->varname);
            return r1 ;
        }
        case READ_NODE :{
            int r1 = getReg() ;
            int effectiveAddress = getAddress(root->left,op);

            auxFunctions(op,READ_CODE,r1,effectiveAddress);
            freeReg();
            freeReg();
            return -1;
        }
        case FUNC_CALL_NODE : {
            int reg ;
            
            for(int i =0;i<=maxReg;i++) {
                fprintf(op, "PUSH R%d\n",i);    //Saving all registers
            }

            struct tnode * currArgs = root->left->argList ;
            while(currArgs) {
                reg = codeGen(currArgs,op); 
                fprintf(op, "PUSH R%d\n",reg);
                freeReg();
                currArgs = currArgs->argList ;
            }

            fprintf(op, "PUSH R0\n");

            fprintf(op, "CALL F%d\n",root->Gentry->flabel);

            reg = getReg();
            fprintf(op, "POP R%d\n",reg);

            int argNumbers = 0;
            struct ParamStruct * params = root->Gentry->paramList ;
            while(params) {
                argNumbers++ ;
                params = params->next ;
            }
            fprintf(op, "SUB SP, %d\n",argNumbers);
            for(int i=maxReg-1; i>=0;i--) {
                fprintf(op, "POP R%d\n",i);
            }
            return reg ;
        }
        case RETURN_NODE : {
            int reg = codeGen(root->left,op);

            int retValAddrReg = getReg();

            fprintf(op, "MOV R%d, BP\n",retValAddrReg);
            fprintf(op, "SUB R%d, 2\n",retValAddrReg);
            fprintf(op, "MOV [R%d], R%d\n",retValAddrReg,reg);
            freeReg();
            freeReg();

            fprintf(op,"MOV SP, BP\n");

            fprintf(op, "POP BP\n");
            if(root->Gentry->flabel == 0) {
                return -1;
            }
            fprintf(op, "RET\n");
            return -1;
        }
        case WRITE_NODE :{
            r1 = getReg();
            r2 = codeGen(root->left,op);

            auxFunctions(op,WRITE_CODE,r1,r2);
            freeReg();
            freeReg();
            return -1;
        }
        case IF_NODE :{
            label1 = getLabel() ;

            r1 = codeGen(root->left,op);
            fprintf(op, "JZ R%d, L%d\n",r1,label1);
            freeReg();

            codeGen(root->mid,op);

            if(root->right) {
                label2 = getLabel();
                fprintf(op,"JMP L%d\n",label2);
            }

            fprintf(op,"L%d:\n",label1);

            if(root->right) {
                codeGen(root->right,op);
                fprintf(op,"L%d:\n",label2);
            }
            return -1 ;
        }
        case WHILE_NODE :{
            label1 = getLabel() ;
            label2 = getLabel() ;

            pushLabel(label2,label1);

            fprintf(op, "L%d:\n",label1);
            r1 = codeGen(root->left,op);
            fprintf(op, "JZ R%d, L%d\n",r1,label2);
            freeReg();

            codeGen(root->right,op);
            fprintf(op, "JMP L%d\n",label1);
            fprintf(op, "L%d:\n",label2);

            popLabel();

            return -1;
        }
        case ASSIGN_NODE :{
            r2 = codeGen(root->right,op);
            int assignReg = getAddress(root->left,op);
            fprintf(op, "MOV [R%d], R%d\n",assignReg,r2);

            freeReg();
            freeReg();
            return -1;
        }
        case DEREF_NODE : {
            int ptrAddress = getAddress(root->left,op);

            fprintf(op,"MOV R%d, [R%d]\n",ptrAddress,ptrAddress);
            fprintf(op,"MOV R%d, [R%d]\n",ptrAddress,ptrAddress);
            return ptrAddress ;
        }
        case ADDR_NODE : {
            return getAddress(root->left,op);
        }
        case ADD_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "ADD R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case MINUS_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "SUB R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case MUL_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "MUL R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case DIV_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "DIV R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case MOD_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op,"MOD R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;  
        }
        case GT_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "GT R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case GE_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "GE R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case LT_NODE :{
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "LT R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case LE_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op,"LE R%d, R%d\n",r1,r2);
            freeReg();
            return r1;
        }
        case EQ_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "EQ R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case NE_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "NE R%d, R%d\n",r1,r2);
            freeReg();
            return r1 ;
        }
        case AND_NODE : {
            label1 = getLabel() ;
            label2 = getLabel();

            r1 = codeGen(root->left,op);
            fprintf(op, "JZ R%d, L%d\n",r1,label1);

            r2 = codeGen(root->right,op);
            fprintf(op,"JZ R%d, L%d\n",r2,label1);
            
            fprintf(op,"MOV R%d, 1\n",r1);
            fprintf(op,"JMP L%d\n",label2);

            fprintf(op,"L%d:\n",label1);
            fprintf(op,"MOV R%d, 0\n",r1);

            fprintf(op,"L%d:\n",label2);

            freeReg();
            return r1;
        }
        case OR_NODE : {
            label1 = getLabel();
            label2 = getLabel();

            r1 = codeGen(root->left,op);
            fprintf(op,"JNZ R%d, L%d\n",r1,label1);

            r2 = codeGen(root->right,op);
            fprintf(op,"JNZ R%d, L%d\n",r2,label1);
            
            fprintf(op,"MOV R%d, 0\n",r1);
            fprintf(op,"JMP L%d\n",label2);

            fprintf(op,"L%d:\n",label1);
            fprintf(op,"MOV R%d, 1\n",r1);

            fprintf(op,"L%d:\n",label2);

            freeReg();
            return r1 ;
        }
        case NOT_NODE : {
            label1 = getLabel();
            label2 = getLabel();

            r1 = codeGen(root->left,op);
            fprintf(op,"JZ R%d, L%d\n",r1,label1);

            fprintf(op,"MOV R%d, 0\n",r1);
            fprintf(op,"JMP L%d\n",label2);

            fprintf(op,"L%d:\n",label1);
            fprintf(op,"MOV R%d, 1\n",r1);

            fprintf(op,"L%d:\n",label2);

            return r1 ;
        }
        case CONNECT_NODE : {
            codeGen(root->left,op);
            codeGen(root->right,op);
            return -1;
        }
        case CONTINUE_NODE :{
            fprintf(op, "JMP L%d\n",getContinueLabel());
            return -1;
        }
        case BREAK_NODE : {
            fprintf(op, "JMP L%d\n",getBreakLabel());
            return -1;
        }
        case EXIT_NODE : {
            auxFunctions(op,EXIT_CODE,-1,-1);
            return -1;
        }
    }
}

void inorder(struct tnode * root) {
    if(root ) {
        inorder(root->left);
        
        switch(root->nodetype) {
            case NUM_NODE : {
                printf("%d ",root->val);
                break;
            }
            case RETURN_NODE : {
                printf("Return ");
                break;
            }
            case EXIT_NODE : {
                printf("exit ");
                break;
            }
            case STR_NODE : {
                printf("%s ",root->varname);
                break;
            }
            case ID_NODE : {
                printf("%s ",root->varname);
                break;
            }
            case ARR_NODE : {
                printf("%s ",root->varname);
            }
            case DEREF_NODE : {
                printf("(*%s) ",root->left->varname);
                break;
            }
            case READ_NODE : {
                printf("Read ");
                break;
            }
            case WRITE_NODE : {
                printf("Write ");
                break;
            }
            case ASSIGN_NODE : {
                printf("= ");
                break;
            }
            case ADD_NODE : {
                printf("+ ");
                break;
            }
            case MOD_NODE : {
                printf("mod ");
                break;
            }
            case MINUS_NODE : {
                printf("- ");
                break;
            }
            case MUL_NODE : {
                printf("* ");
                break;
            }
            case DIV_NODE : {
                printf("/ ");
                break;
            }
            case IF_NODE : {
                printf("if ");
                break;
            }
            case ELSE_NODE : {
                printf("else ");
            }
            case WHILE_NODE : {
                printf("while ");
                break;
            }
            case LT_NODE : {
                printf("< ");
                break;
            }
            case LE_NODE : {
                printf("<= ");
                break;
            }
            case GT_NODE : {
                printf("> ");
                break;
            }
            case GE_NODE : {
                printf(">= ");
                break;
            }
            case NE_NODE : {
                printf("!= ");
                break;
            }
            case EQ_NODE : {
                printf("== ");
                break;
            }
        }
        inorder(root->right);
    }
}
