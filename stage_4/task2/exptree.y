%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exptree.h"
    #include "symbolTable.h"
    #include "codeGen.h"


    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
    int currentType ;

    struct Gsymbol * head  = NULL ;
%}

%union {
    struct tnode * node;
    int number ;
    char * string ;
}

%token PLUS MINUS STAR DIV 
%token LE LT GT GE EQ NE AMPERSAND
%token IF ELSE WHILE REPEAT UNTIL DO BREAK CONTINUE ENDIF ENDWHILE 
%token DECL ENDDECL BEG END INT STR SEMICOLON THEN READ WRITE EXIT_PR

%token <number> NUM 
%token <string> STR_LITERAL
%token <node> ID 

%type <node> program stmt stmtList ipStmt opStmt assignStmt whileStmt doWhileStmt repeatStmt ifStmt
%type <node> IDENTIFIERS expr 


%nonassoc NE EQ LT LE GE GT 
%left PLUS MINUS  
%left STAR DIV MOD 
%right ASSIGN


%%

program     :   Declarations BEG stmtList END SEMICOLON     {
                                                                $$ = $3 ;
                                                                printSymbolTable(head);
                                                                inorder($3);
                                                                printf("\n");
                                                                output = fopen("output.xsm","w");
                                                                auxFunctions(output,ENTRY_CODE,-1,-1);
                                                                codeGen($3,output);
                                                                auxFunctions(output,EXIT_CODE,-1,-1);
                                                                fclose(output);
                                                                exit(0);
                                                            }
            |   Declarations BEG END SEMICOLON              { }
            |   BEG END SEMICOLON                           { }
            ;

stmtList    :   stmtList stmt                   {   $$ = connectNode($1,$2); }
            |   stmt                            {   $$ = $1 ; }
            ;

stmt        :   ipStmt                          {   $$ = $1 ; }
            |   opStmt                          {   $$ = $1 ; }
            |   assignStmt                      {   $$ = $1 ; }
            |   ifStmt                          {   $$ = $1 ; }
            |   whileStmt                       {   $$ = $1 ; }
            |   doWhileStmt                     {   $$ = $1 ; }
            |   repeatStmt                      {   $$ = $1 ; }
            |   BREAK SEMICOLON                 {   $$ = flowControlNode(BREAK_NODE); }
            |   CONTINUE SEMICOLON              {   $$ = flowControlNode(CONTINUE_NODE); }
            |   EXIT_PR SEMICOLON               {   $$ = exitNode(EXIT_NODE); }
            ;                   

ifStmt      :   IF '(' expr ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON     {   
                                                                                    struct tnode * temp = ifelseNode(ELSE_NODE,$6,$8);
                                                                                    $$ = ifelseNode(IF_NODE,$3,temp);
                                                                                }
            |   IF '(' expr ')' THEN stmtList ENDIF SEMICOLON                   {
                                                                                    $$ = ifelseNode(IF_NODE,$3,$6);
                                                                                }
            ;

whileStmt   :    WHILE '(' expr ')' DO stmtList ENDWHILE SEMICOLON              {
                                                                                    $$ = loopNode(WHILE_NODE,$3,$6);
                                                                                }
            ;

doWhileStmt :   DO stmtList WHILE '(' expr ')' SEMICOLON                        {   $$ = loopNode(DOWHILE_NODE,$5,$2); }
            ;

repeatStmt  :   REPEAT stmtList UNTIL '(' expr ')' SEMICOLON                    {   $$ = loopNode(REPEAT_NODE,$5,$2); }
            ;

ipStmt      :   READ '(' IDENTIFIERS ')' SEMICOLON                              {   $$ = ipOpNode(READ_NODE,$3); }
            ;

opStmt      :   WRITE '(' expr ')' SEMICOLON                                    {   $$ = ipOpNode(WRITE_NODE,$3); }
            ;

assignStmt  :   IDENTIFIERS ASSIGN expr SEMICOLON                               {   $$ = assignNode($1,$3); }
            ;

Type        :   INT                                                             {   currentType = INT_TYPE ;}
            |   STR                                                             {   currentType = STR_TYPE ;}
            ;

Declarations:   DECL DeclList ENDDECL                                           { }
            ;


DeclList    :   DeclList Decl                                                   { }
            |   Decl                                                            { }
            ;

Decl        :   Type VarList SEMICOLON                                          { }
            ;

VarList     :   ID '[' NUM ']'                                                  {   head = insertTable(head,($1)->varname,currentType,$3,0);}
            |   ID '[' NUM ']' '[' NUM ']'                                      {   head = insertTable(head,($1)->varname,currentType,$3,$6);}
            |   VarList ',' ID '[' NUM ']'                                      {   head = insertTable(head,($3)->varname,currentType,$5,0);}
            |   VarList ',' ID                                                  {   head = insertTable(head,($3)->varname,currentType,1,0); }
            |   VarList ',' ID '[' NUM ']' '[' NUM ']'                          {   head = insertTable(head,($3)->varname,currentType,$5,$8);}
            |   VarList ',' STAR ID                                             {   head = insertTable(head,($4)->varname,currentType,1,0); }
            |   ID                                                              {   head = insertTable(head,($1)->varname,currentType,1,0); }
            |   STAR ID                                                         {   head = insertTable(head,($2)->varname,currentType == INT_TYPE ? INT_PTR_TYPE : STR_PTR_TYPE,1,0); }
            ;

IDENTIFIERS :   ID                                                              {   
                                                                                    struct tnode * node = setTypeId(head,$1,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   ID '['  expr ']'                                                {   
                                                                                    struct tnode * node = setTypeId(head,$1,$3,NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   ID '['  expr ']' '['  expr ']'                                  {
                                                                                    struct tnode * node = setTypeId(head,$1,$3,$6);
                                                                                    node->nodetype = MATRIX_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   STAR ID                                                         {   
                                                                                    struct tnode * node = setTypeId(head,$2,NULL,NULL);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    $$ = pointerNode(head,DEREF_NODE,node,NULL,NULL);

                                                                                }

            |   AMPERSAND ID                                                    {   
                                                                                    struct tnode * node = setTypeId(head,$2,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = pointerNode(head,ADDR_NODE,node,NULL,NULL);
                                                                                    
                                                                                }

            ;

            
expr        :   expr MINUS expr             {   $$ = exprNode(MINUS_NODE, $1, $3); }
            |   expr PLUS  expr             {   $$ = exprNode(ADD_NODE, $1, $3); }
            |   expr STAR  expr             {   $$ = exprNode(MUL_NODE, $1, $3); }
            |   expr DIV   expr             {   $$ = exprNode(DIV_NODE, $1, $3); }
            |   expr MOD   expr             {   $$ = exprNode(MOD_NODE, $1, $3); }
            |   expr LE    expr             {   $$ = exprNode(LE_NODE, $1, $3); }
            |   expr LT    expr             {   $$ = exprNode(LT_NODE, $1, $3); }
            |   expr GT    expr             {   $$ = exprNode(GT_NODE, $1, $3); }
            |   expr GE    expr             {   $$ = exprNode(GE_NODE, $1, $3); }
            |   expr EQ    expr             {   $$ = exprNode(EQ_NODE, $1, $3); }
            |   expr NE    expr             {   $$ = exprNode(NE_NODE, $1, $3); }
            |   '(' expr ')'                {   $$ = $2 ;}
            |   NUM                         {   $$ = createTreeNode($<number>1,INT_TYPE,NULL,NUM_NODE,NULL,NULL,NULL); }
            |   STR_LITERAL                 {   $$ = createTreeNode(NO_VAL,STR_TYPE,$<string>1,STR_NODE,NULL,NULL,NULL); }
            |   IDENTIFIERS                 {   $$ = $1 ;}
            ;

%%

int yyerror(const char *s) {
    printf("Erorr: %s/n",s);
    return 1;
}

extern FILE* yyin;

int main(int argc, char** argv) {

    if(argc > 1){
        yyin = fopen(argv[1], "r");
        if(!yyin){
            printf("Error opening file\n");
            return 1;
        }
    }

    output = fopen("output.xsm", "w");
    yyparse();

    fclose(output);
    return 0;
}