%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exptree.h"
    #include "symbolTable.h"
    #include "codeGen.h"

    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
    extern int localBinding ;
    int currentType ;
    int paramType ;
    int currLocalType ;
    int flabel = 1;
    struct Gsymbol * Ghead  = NULL ;
    struct Lsymbol * Lhead  = NULL ; 
    struct ParamStruct * paramHead = NULL ;
    struct Gsymbol * currentFunc = NULL ;  //Current function for return type checking 
%}


%union {
    struct ParamStruct * paramList ;
    struct tnode * node;
    int number ;
    char * string ;
}

%token PLUS MINUS STAR DIV 
%token LE LT GT GE EQ NE AND OR NOT ASSIGN
%token IF ELSE WHILE REPEAT UNTIL DO BREAK CONTINUE ENDIF ENDWHILE 
%token DECL ENDDECL BEG END INT STR SEMICOLON THEN READ WRITE EXIT_PR
%token MAIN RETURN 

%token <number> NUM 
%token <string> STR_LITERAL
%token <node> ID 

%type <node> program stmt stmtList ipStmt opStmt assignStmt whileStmt doWhileStmt repeatStmt ifStmt
%type <node> IDENTIFIERS EXPR Ret Body ArgList
%type <paramList> ParamList Param


%right ASSIGN
%left AND OR NOT 
%nonassoc NE EQ LT LE GE GT 
%left PLUS MINUS 
%left STAR DIV  


%%

program     :   GDeclBlock FDefBlock MainBlock  {}
            |   GDeclBlock MainBlock            {}
            |   MainBlock                       {}
            ;

MainBlock   :   INT MAIN '(' ')' '{'            {   
                                                    Ghead = insertTable(Ghead,"F0",INT_TYPE,0,0); 
                                                    currentFunc = find(Ghead, "F0");
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", "main");
                                                        exit(1);
                                                    }
                                                }
                LdeclBlock Body '}'             {

                                                    currentFunc->bodyAST = $8 ;
                                                    currentFunc->Lentry = Lhead ;
                                                    
                                                    funcCodeGen(currentFunc,output); 
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                    
                                                }
            ;

GDeclBlock  :   DECL GDeclList ENDDECL      {}
            |   DECL ENDDECL                {}
            ;

GDeclList   :   GDeclList GDecl             {}
            |   GDecl                       {}
            ;

GDecl       :   GType GidList SEMICOLON     {}
            ;

GidList     :   GidList ',' Gid             {}
            |   Gid                         {}
            ;

Gid         :   ID                          {   Ghead = insertTable(Ghead,($1)->varname,currentType,1,-1); }
            |   ID '[' NUM ']'              {   Ghead = insertTable(Ghead,($1)->varname,currentType,$3,-1);}
            |   ID '(' ParamList ')'        {   
                                                Ghead = insertTable(Ghead,($1)->varname,currentType,0,flabel++);
                                                insertParamListToGST(Ghead,($1)->varname,$3);
                                            }
            ;

FDefBlock   :   FDefBlock Fdef              {}
            |   Fdef                        {}
            ;

Body        :   BEG stmtList Ret END SEMICOLON  {  
                                                    $$ = connectNode($2,$3); 
                                                    //inorder($$);
                                                }
            |   BEG Ret END SEMICOLON           { 
                                                    $$ = $2 ;
                                                }
            ;

ParamList   :   ParamList ',' Param         {   $$ = insertToParamList($1,($3)->name,($3)->type) ; }
            |   Param                       {   $$ = insertToParamList(NULL,($1)->name,($1)->type) ;}
            |                               {   $$ = NULL ;}
            ;

Param       :   ParType ID                  {   $$ = createParamStructNode(strdup($2->varname),paramType); }
            ;

Ret         :   RETURN EXPR SEMICOLON       {   
                                                $$ = returnNode(currentFunc,RETURN_NODE,$2); 
                                                //printf("%s\n",$$->left->varname);
                                            }

LdeclBlock  :   DECL LDeclList ENDDECL      {   localBinding = 1; }
            |   DECL ENDDECL                {}
            ;

LDeclList   :   LDeclList LDecl             {}
            |   LDecl                       {}
            ;

LDecl       :   LType IdList SEMICOLON      {}
            ;

IdList      :   IdList ',' ID               {   Lhead = insertToLocalTable(Lhead,($3)->varname,currLocalType);}
            |   ID                          {   Lhead = insertToLocalTable(Lhead,($1)->varname,currLocalType);}
            ;

GType       :   INT                         {   currentType = INT_TYPE ;}
            |   STR                         {   currentType = STR_TYPE ;}
            ;

ParType     :   INT                         {   paramType = INT_TYPE ;}
            |   STR                         {   paramType = STR_TYPE ;}
            ;

LType       :   INT                         {   currLocalType = INT_TYPE ;}
            |   STR                         {   currLocalType = STR_TYPE ;}
            ;


EXPR        :   EXPR MINUS EXPR             {   $$ = exprNode(MINUS_NODE, $1, $3); }
            |   EXPR PLUS  EXPR             {   $$ = exprNode(ADD_NODE, $1, $3); }
            |   EXPR STAR  EXPR             {   $$ = exprNode(MUL_NODE, $1, $3); }
            |   EXPR DIV   EXPR             {   $$ = exprNode(DIV_NODE, $1, $3); }
            |   EXPR AND   EXPR             {   $$ = exprNode(AND_NODE,$1, $3); }
            |   EXPR OR    EXPR             {   $$ = exprNode(OR_NODE,$1, $3); }
            |   NOT EXPR                    {   $$ = exprNode(NOT_NODE,$2, NULL);}
            |   EXPR LE    EXPR             {   $$ = exprNode(LE_NODE, $1, $3); }
            |   EXPR LT    EXPR             {   $$ = exprNode(LT_NODE, $1, $3); }
            |   EXPR GT    EXPR             {   $$ = exprNode(GT_NODE, $1, $3); }
            |   EXPR GE    EXPR             {   $$ = exprNode(GE_NODE, $1, $3); }
            |   EXPR EQ    EXPR             {   $$ = exprNode(EQ_NODE, $1, $3); }
            |   EXPR NE    EXPR             {   $$ = exprNode(NE_NODE, $1, $3); }
            |   '(' EXPR ')'                {   $$ = $2 ;}
            |   NUM                         {   $$ = createTreeNode($<number>1,INT_TYPE,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
            |   STR_LITERAL                 {   $$ = createTreeNode(NO_VAL,STR_TYPE,$<string>1,STR_NODE,NULL,NULL,NULL,NULL); }
            |   IDENTIFIERS                 {   $$ = $1 ; }
            |   ID '(' ')'                  {   
                                                $$ = funcNode(Ghead,$1,NULL);
                                            }
            |   ID '(' ArgList ')'          {   $$ = funcNode(Ghead,$1,$3);}
            ;

ArgList     :   ArgList ',' EXPR            {
                                                $$ = appendArg($1,$3);
                                            }
            |   EXPR                        {   
                                                $$ = $1;
                                                $$->argList = NULL ;
                                            }
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

Fdef        :   GType ID '(' ParamList ')' '{'  {   
                                                    
                                                    currentFunc = find(Ghead, $2->varname);
                                                    checkDeclAndDefn(currentFunc,currentType);
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", $2->varname);
                                                        exit(1);
                                                    }
                                                    compareParamList(currentFunc,$4) ;
                                                    Lhead = createLSTFromParams($4);
                                                }
                LdeclBlock Body '}'             {
                                                    currentFunc->bodyAST = $9 ;
                                                    currentFunc->Lentry = Lhead ;
                                                    //printLocalSymbolTable(Lhead);
                                                    
                                                    funcCodeGen(currentFunc,output);                                       
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                }
            ;



ifStmt      :   IF '(' EXPR ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON     {   
                                                                                    
                                                                                    $$ = ifelseNode(IF_NODE,$3,$6,$8);
                                                                                }
            |   IF '(' EXPR ')' THEN stmtList ENDIF SEMICOLON                   {
                                                                                    
                                                                                    $$ = ifelseNode(IF_NODE,$3,$6,NULL);
                                                                                }
            ;

whileStmt   :    WHILE '(' EXPR ')' DO stmtList ENDWHILE SEMICOLON              {
                                                                                    $$ = loopNode(WHILE_NODE,$3,$6);
                                                                                }
            ;

doWhileStmt :   DO stmtList WHILE '(' EXPR ')' SEMICOLON                        {   $$ = loopNode(DOWHILE_NODE,$5,$2); }
            ;

repeatStmt  :   REPEAT stmtList UNTIL '(' EXPR ')' SEMICOLON                    {   $$ = loopNode(REPEAT_NODE,$5,$2); }
            ;

ipStmt      :   READ '(' IDENTIFIERS ')' SEMICOLON                              {   $$ = ipOpNode(READ_NODE,$3); }
            ;

opStmt      :   WRITE '(' EXPR ')' SEMICOLON                                    {   $$ = ipOpNode(WRITE_NODE,$3); }
            ;

assignStmt  :   IDENTIFIERS ASSIGN EXPR SEMICOLON                               {   $$ = assignNode($1,$3); }
            ;

IDENTIFIERS :   ID                                                              {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$1,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   ID '['  EXPR ']'                                                {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$1,$3,NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    $$ = node;
                                                                                }
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

    auxFunctions(output,ENTRY_CODE,-1,-1);
    fprintf(output,"CALL F0\n");
    yyparse();
    printSymbolTable(Ghead);
    auxFunctions(output,EXIT_CODE,-1,-1);

    fclose(output);
    return 0;
}