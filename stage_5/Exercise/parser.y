%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "exptree.h"
    #include "symbolTable.h"
    #include "TypeTable.h"
    #include "codeGen.h"

    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
    extern int yylineNum; 
    extern int localBinding ;
    int flabel = 1;
    bool isPointer = false ;

    struct Gsymbol * Ghead  = NULL ;
    struct Lsymbol * Lhead  = NULL ; 
    struct ParamStruct * paramHead = NULL ;
    struct Gsymbol * currentFunc = NULL ;  //Current function for return type checking 

    struct TypeTable * currentType ;
    struct TypeTable * paramType ;
    struct TypeTable * currLocalType ;

    struct TypeTable * TTableHead = NULL ;
    struct TypeTable * currTupleType = NULL ;
%}


%union {
    struct ParamStruct * paramList ;
    struct tnode * node;
    int number ;
    char * string ;
    struct Fieldlist * flist ;
    struct TypeTable * type ;
}

%token PLUS MINUS STAR DIV AMPERSAND
%token LE LT GT GE EQ NE AND OR NOT ASSIGN
%token IF ELSE WHILE REPEAT UNTIL DO BREAK CONTINUE ENDIF ENDWHILE 
%token DECL ENDDECL BEG END INT STR SEMICOLON THEN READ WRITE EXIT_PR
%token MAIN RETURN 
%token TUPLE DOT 

%token <number> NUM 
%token <string> STR_LITERAL
%token <node> ID 

%type <node> program stmt stmtList ipStmt opStmt assignStmt whileStmt doWhileStmt repeatStmt ifStmt
%type <node> IDENTIFIERS EXPR Ret Body ArgList
%type <paramList> ParamList Param
%type <flist> FieldDefList FieldDef 


%right ASSIGN
%left AND OR NOT 
%nonassoc NE EQ LT LE GE GT 
%left PLUS MINUS 
%left STAR DIV  
%left DOT 

%%

program     :   GDeclBlock FDefBlock MainBlock  {}
            |   GDeclBlock MainBlock            {}
            |   MainBlock                       {}
            ;

MainBlock   :   INT MAIN '(' ')' '{'            {   
                                                    Ghead = insertTable(Ghead,"F0",T_INT,false,0,0); 
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
            |   GTupleDecl SEMICOLON        {}
            ;

GTupleDecl  :   TUPLE ID '(' FieldDefList ')'   {
                                                    int size = 0;
                                                    struct Fieldlist * temp = $4 ;
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next ;
                                                    }
                                                    TTableHead = TInstall(TTableHead,$2->varname,size,$4);
                                                    currTupleType = TLookup(TTableHead, $2->varname);

                                                    if(currTupleType == NULL) {
                                                        printf("Tuple Type installation failed\n");
                                                        exit(1);
                                                    }
                                                }
                GTupleIdList                    {   currTupleType = NULL ;}
            ;

GTupleIdList:  GTupleIdList ',' GTupleId       {}
            |  GTupleId                        {}
            ;

GTupleId    :   ID                          {    Ghead = insertTable(Ghead, ($1)->varname, currTupleType, false, currTupleType->size, -1);}
            |   STAR ID                     {    Ghead = insertTable(Ghead, ($2)->varname, currTupleType, true, currTupleType->size, -1);}
            ;

FieldDefList:   FieldDefList ',' FieldDef   {   
                                                struct Fieldlist * temp = $1 ;
                                                while(temp->next) {
                                                    temp = temp->next ;
                                                }
                                                $3->fieldIndex = temp->fieldIndex + 1;
                                                temp->next = $3 ;
                                                $$ = $1 ;
                                            }
            |   FieldDef                    {   
                                                $1->fieldIndex = 0; 
                                                $$ = $1 ;
                                            }
            ;

FieldDef    :   GType ID                    {   $$ = createFieldlistNode(($2)->varname,currentType,0); }
            ;

GidList     :   GidList ',' Gid             {}
            |   Gid                         {}
            ;

Gid         :   ID                          {   Ghead = insertTable(Ghead,($1)->varname,currentType,false,1,-1); }
            |   ID '[' NUM ']'              {   Ghead = insertTable(Ghead,($1)->varname,currentType,false,$3,-1);}
            |   ID '(' ParamList ')'        {   
                                                Ghead = insertTable(Ghead,($1)->varname,currentType,false,0,flabel++);
                                                insertParamListToGST(Ghead,($1)->varname,$3);
                                            }
            |   STAR ID                     {   Ghead = insertTable(Ghead,($2)->varname,currentType == T_INT ? T_INT_PTR : T_STR_PTR,true,1,-1); }
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

ParamList   :   ParamList ',' Param         {   $$ = insertToParamList($1,($3)->name,($3)->type,($3)->isPtr) ; }
            |   Param                       {   $$ = insertToParamList(NULL,($1)->name,($1)->type,($1)->isPtr) ;}
            |                               {   $$ = NULL ;}
            ;

Param       :   ParType ID                  {   $$ = createParamStructNode(strdup($2->varname),paramType,false); }
            |   ParType STAR ID             {   $$ = createParamStructNode(strdup($3->varname),paramType == T_INT ? T_INT_PTR : T_STR_PTR,true); }
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
            |   LTupleDecl SEMICOLON        {}  
            ;

LTupleDecl  :   TUPLE ID '(' FieldDefList ')'   {
                                                    int size = 0;
                                                    struct Fieldlist *temp = $4;
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next;
                                                    }
                                                    TTableHead = TInstall(TTableHead, $2->varname, size, $4);
                                                    currTupleType = TLookup(TTableHead, $2->varname);
                                                    if (currTupleType == NULL) {
                                                        yyerror("Tuple type installation failed");
                                                        exit(1);
                                                    }
                                                }
                LTupleIdList                    {   currTupleType = NULL ;}
            ;
    
LTupleIdList:   LTupleIdList ',' LTupleId       {}
            |   LTupleId                        {}
            ;

LTupleId    :   ID                          {   Lhead = insertToLocalTable(Lhead, ($1)->varname, currTupleType,false,currTupleType->size);}
            |   STAR ID                     {   Lhead = insertToLocalTable(Lhead, ($2)->varname, currTupleType,true,1);}
            ;


IdList      :   IdList ',' ID               {   Lhead = insertToLocalTable(Lhead,($3)->varname,currLocalType,false,1);}
            |   ID                          {   Lhead = insertToLocalTable(Lhead,($1)->varname,currLocalType,false,1);}
            ;

GType       :   INT                         {   currentType = T_INT ;}
            |   STR                         {   currentType = T_STR ;}
            ;

ParType     :   INT                         {   paramType = T_INT ;}
            |   STR                         {   paramType = T_STR ;}
            ;

LType       :   INT                         {   currLocalType = T_INT ;}
            |   STR                         {   currLocalType = T_STR ;}
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
            |   NUM                         {   $$ = createTreeNode(0,$<number>1,T_INT,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
            |   STR_LITERAL                 {   $$ = createTreeNode(0,NO_VAL,T_STR,$<string>1,STR_NODE,NULL,NULL,NULL,NULL); }
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
            |   STAR ID                                                         {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$2,NULL,NULL);
                                                                                    printf("%s\n",node->varname);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    $$ = pointerNode(Ghead,DEREF_NODE,node,NULL,NULL);
                                                                                }      
            |   AMPERSAND ID                                                    {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$2,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = pointerNode(Ghead,ADDR_NODE,node,NULL,NULL);
                                                                                }
            |   IDENTIFIERS DOT ID                                              {
                                                                                    if ($1->type->fields == NULL) {
                                                                                        yyerror("Field access ('.') on a non-tuple type");
                                                                                        exit(1);
                                                                                    }
                                                                                    struct Fieldlist *field = FLookup($1->type, $3->varname);

                                                                                    if (field == NULL) {
                                                                                        printf("Field '%s' not found in tuple type '%s'", $3->varname, $1->type->name);                                                                                   
                                                                                        exit(1);
                                                                                    }
                                                                                    $$ = createTreeNode(0,field->fieldIndex, field->type, field->name, FIELD_NODE, NULL, $1, NULL, NULL);
                                                                                }                                                              
            ;

%%

int yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineNum);
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

    TTableHead = createTypeTable(); // TInstall default types -> int, str, void, bool

    auxFunctions(output,ENTRY_CODE,-1,-1);
    fprintf(output,"CALL F0\n");
    yyparse();
    auxFunctions(output,EXIT_CODE,-1,-1);

    fclose(output);
    return 0;
}