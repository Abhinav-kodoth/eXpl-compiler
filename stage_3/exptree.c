#include "exptree.h"

#define STACK_SIZE 100

int breakStack[STACK_SIZE];
int continueStack[STACK_SIZE];
int stackTop = -1;

int maxReg = -1;
int maxLabel = -1;

int checkOperator(int nodeType) {
    return (nodeType == ADD_NODE || nodeType == MUL_NODE || nodeType == MINUS_NODE || 
            nodeType == DIV_NODE || nodeType == NE_NODE || nodeType == EQ_NODE ||
            nodeType == LT_NODE || nodeType == LE_NODE || nodeType == GT_NODE ||
            nodeType == GE_NODE || nodeType == ASSIGN_NODE);
}


struct tnode * createTreeNode(int val, int type, char* varname, int nodetype, struct tnode *l, struct tnode *r) {
    struct tnode * temp = malloc(sizeof(struct tnode));

    if(checkOperator(nodetype) &&  (( l && l->type != INT_TYPE) || (r &&  r->type != INT_TYPE ))) {
        perror("type mismatch");
        exit(1);
    }

    temp->val = val ;
    temp->varname = varname ;

    temp->type = type ;
    temp->nodetype = nodetype ;

    temp->left = l ;
    temp->right = r ;

    return temp ;
}


void inorder(struct tnode * root) {
    if(root ) {
        inorder(root->left);
        
        switch(root->nodetype) {
            case NUM_NODE : {
                printf("%d ",root->val);
                break;
            }
            case ID_NODE : {
                printf("%c ",root->varname[0]);
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

int evaluate(struct tnode * root) {
    if(root == NULL) {
        return -1 ;
    }
    switch(root->nodetype) {
        case NUM_NODE : {
            return root->val ;
        }

        case ID_NODE : {
            return nums[root->varname[0] - 'a'] ;
        }

        case READ_NODE : {
            scanf("%d",&nums[root->left->varname[0] - 'a']);
            break ;
        }
        case WRITE_NODE : {
            printf("%d\n",evaluate(root->left));
            break;
        }

        case ADD_NODE : {
            return evaluate(root->left) + evaluate(root->right) ;
        }
        case MINUS_NODE : {
            return evaluate(root->left) - evaluate(root->right) ;
        }
        case DIV_NODE : {
            return evaluate(root->left) / evaluate(root->right) ;
        }
        case MUL_NODE : {
            return evaluate(root->left) * evaluate(root->right) ;
        }
        case NE_NODE : {
            return (evaluate(root->left) != evaluate(root->right)) ;
        }
        case EQ_NODE : {
            return evaluate(root->left) == evaluate(root->right) ;
        }
        case GT_NODE : {
            return evaluate(root->left) > evaluate(root->right) ;
        }
        case GE_NODE : {
            return evaluate(root->left) >= evaluate(root->right) ;
        }
        case LE_NODE : {
            return evaluate(root->left) <= evaluate(root->right) ;
        }
        case LT_NODE : {
            return evaluate(root->left) < evaluate(root->right) ;
        }
        case ASSIGN_NODE : {
            nums[root->left->varname[0] - 'a'] = evaluate(root->right) ;
            return 0;
        }

        case CONNECT_NODE : {
            int res = evaluate(root->left);
            if(res == EVAL_BREAK || res == EVAL_CONTINUE) {
                return res ;
            }

            res = evaluate(root->right);
            if(res == EVAL_BREAK || res == EVAL_CONTINUE) {
                return res ;
            }
            return EVAL_NORMAL;
        }

        case IF_NODE : {
            if(evaluate(root->left)) {
                if(root->right->nodetype == ELSE_NODE) {
                    return evaluate(root->right->left) ;
                } else {
                    return evaluate(root->right) ;
                }
            } else {
                if(root->right->nodetype == ELSE_NODE) {
                    return evaluate(root->right->right);
                } 
            }
            break;
        }
        case WHILE_NODE : {
            while(evaluate(root->left)) {
                int res = evaluate(root->right) ;
                if(res == EVAL_BREAK) {
                    break;
                } else if(res == EVAL_CONTINUE) {
                    continue;
                } else if(res != EVAL_NORMAL) {
                    return res ;
                }
            } 
            return EVAL_NORMAL ;
            break ;
        }
        case DOWHILE_NODE : {
            do {
                int res = evaluate(root->right);
                if(res == EVAL_BREAK) {
                    break;
                } else if(res == EVAL_CONTINUE) {
                    continue; 
                } else if(res != EVAL_NORMAL) {
                    return res ;
                }

            } while(evaluate(root->left));
            break;
        }
        case REPEAT_NODE : {
            do {
                int res = evaluate(root->right);
                if(res == EVAL_BREAK) {
                    break;
                } else if(res == EVAL_CONTINUE) {
                    continue;
                } else if(res != EVAL_NORMAL) {
                    return res ;
                }
            } while (!evaluate(root->left));
            return EVAL_NORMAL ;
            break; 
        }
        case BREAK_NODE : {
            return EVAL_BREAK ;
        }
        case CONTINUE_NODE : {
            return EVAL_CONTINUE ;
        }
    }
    return 0 ;
}

int getLabel() {
    return ++maxLabel ;
}

int getReg() {
    if(maxReg < 16) {
        return ++maxReg ;
    } else {
        perror("Out of Registers");
    }
    return -1 ;
}

void freeReg() {
    if(maxReg >= 0) {
        maxReg-- ;
    }
}

void pushLabel(int breakLabel, int continueLabel) {
    if(stackTop + 1 >= STACK_SIZE) {
        perror("Too many nested loops");
        exit(1);
    }
    stackTop++ ;
    breakStack[stackTop] = breakLabel ;
    continueStack[stackTop] = continueLabel ;
}

void popLabel() {
    if(stackTop < 0) {
        perror("Popping from empty stack");
        exit(1);
    }
    stackTop-- ;
}

int getBreakLabel() {
    return breakStack[stackTop];
}

int getContinueLabel() {
    return continueStack[stackTop];
}


void auxFunctions(FILE * op, int codePrint, int reg1, int reg2) {
    switch(codePrint) {
        case ENTRY_CODE : {
            fprintf(op, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",0,2056,0,0,0,0,0,0);
            fprintf(op, "MOV SP, %d\n",4096 + (int)('z' - 'a')); //Reserving var for stack from a-z
            break;
        }
        case EXIT_CODE : {
            fprintf(op, "MOV R0, \"Exit\"\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "PUSH R0\n");
            fprintf(op, "CALL 0\n");
            break;
        }
        case READ_CODE : {
            fprintf(op, "MOV R%d, \"Read\"\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, -1\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, R%d\n", reg1, reg2);
            fprintf(op, "PUSH R%d\n",reg1);
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
        case WRITE_CODE : {
            fprintf(op, "MOV R%d, \"Write\"\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, -2\n",reg1);
            fprintf(op, "PUSH R%d\n",reg1);
            fprintf(op, "MOV R%d, R%d\n", reg1, reg2);
            fprintf(op, "PUSH R%d\n",reg1);
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

int codeGen(struct tnode * root, FILE * op) {
    int r1, r2, idPos, label1, label2 ;

    switch(root->nodetype) {
        case NUM_NODE : {
            r1 = getReg() ;
            fprintf(op, "MOV R%d, %d\n",r1,root->val);
            return r1 ;
        }
        case ID_NODE : {
            r1 = getReg() ;
            idPos = (root->varname[0] - 'a') + 4096 ;
            fprintf(op, "MOV R%d, %d\n",r1,idPos);
            fprintf(op, "MOV R%d, [R%d]\n",r1,r1);
            return r1 ;
        }
        case ASSIGN_NODE : {
            r1 = getReg() ;
            r2 = codeGen(root->right,op);
            idPos = (root->left->varname[0] - 'a') + 4096 ;
            fprintf(op, "MOV R%d, %d\n",r1,idPos);
            fprintf(op, "MOV [R%d], R%d\n", r1, r2);
            freeReg();
            freeReg();
            return -1;
        }
        case READ_NODE : {
            r1 = getReg() ;
            r2 = getReg() ;
            idPos = 4096 + root->left->varname[0] - 'a' ;
            fprintf(op, "MOV R%d, %d\n",r2, idPos);
            auxFunctions(op,READ_CODE,r1,r2);
            freeReg();
            freeReg();
            return -1 ;
        }
        case WRITE_NODE : {
            if(root->left->nodetype == NUM_NODE) {
                r1 = getReg() ;
                r2 = getReg() ;
                fprintf(op, "MOV R%d, %d\n",r2, root->left->val);
                auxFunctions(op,WRITE_CODE,r1,r2);
                freeReg();
            } else if(root->left->nodetype == ID_NODE) {
                r1 = getReg() ;
                r2 = getReg() ;
                idPos = 4096 + root->left->varname[0] - 'a' ;
                fprintf(op, "MOV R%d, %d\n",r2,idPos);
                fprintf(op, "MOV R%d, [R%d]\n", r2, r2);
                auxFunctions(op,WRITE_CODE,r1,r2);
                freeReg();
            } else {
                r1 = getReg() ;
                r2 = codeGen(root->left,op);
                auxFunctions(op,WRITE_CODE,r1,r2);
                freeReg();
            }
            freeReg();
            return -1;
        }
        case ADD_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "ADD R%d , R%d\n", r1, r2);
            freeReg();
            return r1 ;
        }
        case MINUS_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "SUB R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case MUL_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "MUL R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case DIV_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "DIV R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case GT_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "GT R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case GE_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "GE R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case LT_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "LT R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case LE_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "LE R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case EQ_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "EQ R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case NE_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            fprintf(op, "NE R%d, R%d\n", r1,r2);
            freeReg();
            return r1 ;
        }
        case CONNECT_NODE : {
            r1 = codeGen(root->left,op);
            r2 = codeGen(root->right,op);
            freeReg();
            freeReg();
            return -1 ;
        }
        case IF_NODE : {
            label1 = getLabel();

            r1 = codeGen(root->left, op);
            fprintf(op, "JZ R%d, L%d\n", r1, label1);
            freeReg();

            if(root->right->nodetype == ELSE_NODE) {
                label2 = getLabel();
                r1 = codeGen(root->right->left, op);
                fprintf (op, "JMP L%d\n", label2);
                fprintf (op, "L%d:\n", label1);
                freeReg();

                r1 = codeGen(root->right->right, op);
                fprintf (op, "L%d:\n", label2);
                freeReg();
            } else {
                r1 = codeGen(root->right, op);
                freeReg();
                fprintf (op, "L%d:\n", label1);
            }
            return -1;
        }
        case WHILE_NODE : {
            label1 = getLabel();
            label2 = getLabel();
            
            pushLabel(label2,label1); //Since breakLabel->label2 and continueLabel->label1

            fprintf(op, "L%d:\n",label1);
            r1 = codeGen(root->left,op);
            fprintf(op, "JZ R%d, L%d\n", r1, label2);
            freeReg();

            codeGen(root->right,op);

            fprintf(op, "JMP L%d\n", label1);
            fprintf(op, "L%d:\n", label2);
            
            popLabel();

            return -1 ;
        }
        case DOWHILE_NODE : {
            label1 = getLabel();
            label2 = getLabel();

            pushLabel(label2,label1);

            fprintf(op, "L%d:\n", label1) ;
            r1 = codeGen(root->right,op);
            freeReg();

            r1 = codeGen(root->left, op);
            fprintf(op, "JNZ R%d, L%d\n", r1, label1);
            fprintf(op, "L%d:\n", label2);
            freeReg();  
            
            popLabel();

            return -1;
        }
        case REPEAT_NODE : {
            label1 = getLabel();
            label2 = getLabel();

            pushLabel(label2,label1);

            fprintf(op, "L%d:\n", label1) ;
            r1 = codeGen(root->right,op);
            freeReg();

            r1 = codeGen(root->left, op);
            fprintf(op, "JZ R%d, L%d\n", r1, label1);
            fprintf(op, "L%d:\n", label2);
            freeReg();

            pushLabel(label2,label1);

            return -1;
        }
        case CONTINUE_NODE : {
            fprintf(op, "JMP L%d\n",getContinueLabel());
            return -1 ;
        }   
        case BREAK_NODE : {
            fprintf(op, "JMP L%d\n",getBreakLabel());
            return -1;
        }
    }
}