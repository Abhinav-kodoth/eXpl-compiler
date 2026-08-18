%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exptree.h"

    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
%}

%union {
    struct tnode * node;
}

%token PLUS MINUS MUL DIV 
%token BEG END READ WRITE ASSIGN SEMICOLON 
%token IF THEN WHILE ELSE ENDIF ENDWHILE CONTINUE BREAK REPEAT UNTIL
%token NE EQ GT LT GE LE DO 

%token <node> NUM ID

%type <node> expr assignStmt opStmt ipStmt 
%type <node> stmt stmtList ifStmt whileStmt 
%type <node> doWhileStmt repeatStmt


%left NE EQ GT LT GE LE
%left PLUS MINUS 
%left MUL DIV MOD 

%%

program     :   BEG stmtList END SEMICOLON      {
                                                    evaluate($2);
                                                    printf("\n");
                                                    //inorder($2);
                                                    output = fopen("output.xsm","w");
                                                    auxFunctions(output,ENTRY_CODE,-1,-1);
                                                    codeGen($2,output);
                                                    auxFunctions(output,EXIT_CODE,-1,-1);
                                                    fclose(output);
                                                    exit(0);

                                                }
            |   BEG END SEMICOLON               {
                                                    printf("\n");
                                                    exit(0);
                                                }
            ;

stmtList    :   stmtList stmt                   {   $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,CONNECT_NODE,$1,$2); }
            |   stmt                            {   $$ = $1 ;}
            ;

stmt        :   ipStmt                          {   $$ = $1 ;}
            |   opStmt                          {   $$ = $1 ;}
            |   assignStmt                      {   $$ = $1 ;}
            |   ifStmt                          {   $$ = $1 ;}
            |   whileStmt                       {   $$ = $1 ;}
            |   doWhileStmt                     {   $$ = $1 ;}
            |   repeatStmt                      {   $$ = $1 ;}
            |   BREAK SEMICOLON                 {   $$ = createTreeNode(NO_VAL, NO_TYPE,NULL,BREAK_NODE,NULL,NULL) ;}
            |   CONTINUE SEMICOLON              {   $$ = createTreeNode(NO_VAL, NO_TYPE,NULL,CONTINUE_NODE,NULL,NULL) ;}
            ;

ifStmt      :   IF '(' expr ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON     {   
                                                                                    if($3->type == INT_TYPE) {
                                                                                        printf("Type Mismatch\n");
                                                                                        exit(1);
                                                                                    }
                                                                                    struct tnode * temp = createTreeNode(NO_VAL,NO_TYPE,NULL,ELSE_NODE,$6,$8);
                                                                                    $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,IF_NODE,$3,temp);
                                                                                }
            |   IF '(' expr ')' THEN stmtList ENDIF SEMICOLON                   {   
                                                                                    if($3->type== INT_TYPE) {
                                                                                        printf("Type Mismatch\n");
                                                                                        exit(1);
                                                                                    }
                                                                                    $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,IF_NODE,$3,$6);
                                                                                }
            ;

whileStmt   :   WHILE '(' expr ')' DO stmtList ENDWHILE SEMICOLON               {
                                                                                    $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,WHILE_NODE,$3,$6);
                                                                                }
            ;

doWhileStmt :  DO stmtList WHILE '(' expr ')' SEMICOLON                         {
                                                                                    $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,DOWHILE_NODE,$5,$2);
                                                                                }
            ;

repeatStmt  : REPEAT stmtList UNTIL '(' expr ')' SEMICOLON                      {
                                                                                    $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,REPEAT_NODE,$5,$2);
                                                                                }

ipStmt      :   READ '(' ID ')' SEMICOLON       {   $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,READ_NODE,$3,NULL) ;}
            ;

opStmt      :   WRITE '(' expr ')' SEMICOLON    {   $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,WRITE_NODE,$3,NULL) ;}
            ;

assignStmt  :   ID ASSIGN expr SEMICOLON        {   $$ = createTreeNode(NO_VAL,NO_TYPE,NULL,ASSIGN_NODE,$1, $3) ;}
            ;

expr        :   expr PLUS  expr                 {   $$ = createTreeNode(NO_VAL,INT_TYPE,NULL,ADD_NODE, $1, $3); }
            |   expr MINUS expr                 {   $$ = createTreeNode(NO_VAL,INT_TYPE,NULL,MINUS_NODE, $1, $3); }
            |   expr MUL   expr                 {   $$ = createTreeNode(NO_VAL,INT_TYPE,NULL,MUL_NODE, $1, $3); }
            |   expr DIV   expr                 {   $$ = createTreeNode(NO_VAL,INT_TYPE,NULL,DIV_NODE, $1, $3); }
            |   expr NE    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,NE_NODE, $1, $3); }
            |   expr EQ    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,EQ_NODE,$1,$3); }
            |   expr LT    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,LT_NODE,$1,$3); }
            |   expr LE    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,LE_NODE,$1,$3); }
            |   expr GT    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,GT_NODE,$1,$3); }
            |   expr GE    expr                 {   $$ = createTreeNode(NO_VAL,BOOL_TYPE,NULL,GE_NODE,$1,$3); }
            |   '(' expr ')'                    {   $$ = $2 ;}
            |   NUM                             {   $$ = $1 ;}
            |   ID                              {   $$ = $1 ;}
            ;
            
%%

extern FILE * yyin ;

int  yyerror(const char * s) {
    printf("Error : %s\n",s);
    return 1;
}

int main(int argc, char ** argv) {
    if(argc > 1) {
        yyin = fopen(argv[1], "r");
        if(!yyin) {
            printf("Error opening file\n");
            return 1;
        }
    }

    output = fopen("output.xsm", "w");
    yyparse();

    fclose(output);
    return 0;
}
