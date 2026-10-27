%{
    #include "tree/tree.h"
    #include "symbol_table/gst.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include "translate/translate.h"
    #include "type_table/tt.h"
    int yylex();  
    void yyerror(const char *s);
    void postorder(node* root);
    int codeGen(node* root, FILE* targetFile);
    void print(int reg, FILE* targetFile);
    void printAST(node* root, const char* prefix, int isLast);
    void semantics(node* root);
    node* root;
    extern FILE* yyin;
    extern int yylineno;
    extern gst* gstRoot;
%}

%define parse.error verbose

%union {
    struct node* p;
}

%token <p> START END READ WRITE NUM ID GE LE EQ NE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK 
%token <p> CONTINUE BRKP DECL ENDDECL INT STR STRVAL NULLVAL RETURN MAIN TYPE ENDTYPE INITIALIZE 
%token <p> FREE ALLOC CLASS ENDCLASS NEW

%type <p> expr stmt stmtList inputStmt outputStmt assignStmt start ifStmt whileStmt doWhileStmt 
%type <p> gDeclBlock gDeclList gDecl gVarList identifier gVar index ptr fDefBlock mainBlock
%type <p> paramList param fDef lDeclBlock lDeclList lDecl lVarList lVar fBody retStmt argList arg
%type <p> typeDefBlock typeDefList typeDef typeName fieldDeclList fieldDecl field initializeStmt
%type <p> freeStmt classDefBlock classDefList classDef cName cFieldList cField cMethodList cMethod
%type <p> cMethodDefList fieldFn fCallStmt

%nonassoc '<' '>' GE LE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right PTR

%%

start : typeDefBlock classDefBlock gDeclBlock fDefBlock mainBlock   
        { 
            root = makeConnectorNode(
                makeConnectorNode(
                    makeConnectorNode($1, $2), 
                    makeConnectorNode($3, $4)
                ), 
            $5
            ); 
        }
      | typeDefBlock classDefBlock gDeclBlock mainBlock
        { 
            root = makeConnectorNode(
                makeConnectorNode($1, $2), 
                makeConnectorNode($3, $4)
            ); 
        }
      ;

// ----------------------TYPE DEFINITION---------------------------------------

typeDefBlock : TYPE typeDefList ENDTYPE { $$ = $2; }
             | TYPE ENDTYPE             { $$ = NULL; }
             |                          { $$ = NULL; }
             ;

typeDefList : typeDefList typeDef   { $$ = makeConnectorNode($1, $2); }
            | typeDef               { $$ = $1; }
            ;

typeDef : ID '{' fieldDeclList '}'  { $$ = makeTypeDefNode($1, $3); }
        ;

fieldDeclList : fieldDeclList fieldDecl   { $$ = makeConnectorNode($1, $2); }
              | fieldDecl                 { $$ = $1; }
              ;

fieldDecl : typeName ID ';'         { $$ = makeFieldDeclNode($1, $2); }
          ;

typeName : INT   { $$ = makeLeafIdNode("int"); }  // Create makeTypeNode ig?
         | STR   { $$ = makeLeafIdNode("str"); }
         | ID    { $$ = $1; }
         ;

// -----------------------CLASS DEFINITION--------------------------------------

classDefBlock : CLASS classDefList ENDCLASS { $$ = $2; }
              | CLASS ENDCLASS              { $$ = NULL; }
              |                             { $$ = NULL; }

classDefList : classDefList classDef   { $$ = makeConnectorNode($1, $2); }
             | classDef               { $$ = $1; }
             ;

classDef : cName '{' DECL cFieldList cMethodList ENDDECL cMethodDefList '}' { $$ = makeClassDefNode($1, $4, $5, $7); }
classDef : cName '{' DECL cFieldList ENDDECL cMethodDefList '}' { $$ = makeClassDefNode($1, $4, NULL, $6); }
         ;

cName : ID    { $$ = $1; }

cFieldList : cFieldList cField   { $$ = makeConnectorNode($1, $2); }
           | cField              { $$ = $1; }
           ;

cField : typeName ID ';' { $$ = makeClassFieldNode($1, $2); }
       ;

cMethodList : cMethodList cMethod   { $$ = makeConnectorNode($1, $2); }
            | cMethod               { $$ = $1; }
            ;

cMethod : typeName ID '(' paramList ')' ';' { $$ = makeClassMethodNode($1, $2, $4); }
        | typeName ID '(' ')' ';' { $$ = makeClassMethodNode($1, $2, NULL); }
        ;

cMethodDefList : fDefBlock { $$ = makeClassFnDefNode($1); }
               |           { $$ = NULL; }
               ;

// --------------------------GLOBAL DECLARATIONS-----------------------------------

gDeclBlock : DECL gDeclList ENDDECL  { $$ = $2; }
           | DECL ENDDECL            { $$ = NULL; }
           | /* empty */             { $$ = NULL; }
           ;

gDeclList : gDeclList gDecl    { $$ = makeConnectorNode($1, $2); }
          | gDecl              { $$ = $1; }
          ;

gDecl : typeName gVarList ';'   { $$ = makeDeclNode($1, $2); }
      ;

gVarList : gVarList ',' gVar   { $$ = makeConnectorNode($1, $3); }
         | gVar                 { $$ = $1; }
         ;

gVar : ID                   { $$ = $1; }
     | ID '[' NUM ']'       { $$ = makeArrayNode($1, $3); }
     | ID '(' paramList ')' { $$ = makeFnDeclNode($1, $3); }
     | ID '(' ')'           { $$ = makeFnDeclNode($1, NULL); }
     | ptr                  { $$ = $1; }
     ;

paramList : paramList ',' param   { $$ = makeConnectorNode($1, $3); }
          | param                 { $$ = $1; }
          ;

param : typeName ID   { $$ = makeParamNode($1, $2); }
      | typeName ptr  { $$ = makeParamNode($1, $2); }  
      ;

// --------------------------FUNCTION DEFINITIONS-----------------------------------

fDefBlock : fDefBlock fDef    { $$ = makeConnectorNode($1, $2); }
          | fDef              { $$ = $1; }
          ;

fDef : typeName ID '(' paramList ')' '{' lDeclBlock fBody '}'   { $$ = makeFnDefNode($1, $2, $4, $7, $8); }
     | typeName ID '(' ')' '{' lDeclBlock fBody '}'             { $$ = makeFnDefNode($1, $2, NULL, $6, $7); }

lDeclBlock : DECL lDeclList ENDDECL  { $$ = $2; }
           | DECL ENDDECL            { $$ = NULL; }
           | /* empty */             { $$ = NULL; }
           ;

lDeclList : lDeclList lDecl    { $$ = makeConnectorNode($1, $2); }
          | lDecl              { $$ = $1; }
          ;

lDecl : typeName lVarList ';'   { $$ = makeLDeclNode($1, $2); }
      ;

lVarList : lVarList ',' lVar   { $$ = makeConnectorNode($1, $3); }
         | lVar                 { $$ = $1; }
         ;

lVar : ID                   { $$ = $1; }  //No arrays within functions
     | ptr                  { $$ = $1; }
     ;

fBody : START stmtList retStmt END  { $$ = makeConnectorNode($2, $3); }
      ;

retStmt : RETURN expr ';'  { $$ = makeReturnNode($2); }
        ;

stmtList : /* empty */       { $$ = NULL; }
         | stmtList stmt     { $$ = ($1 == NULL) ? $2 : makeConnectorNode($1, $2); }
         ;

stmt : inputStmt             { $$ = $1; }
     | outputStmt            { $$ = $1; }
     | assignStmt            { $$ = $1; }
     | initializeStmt        { $$ = $1; }
     | freeStmt              { $$ = $1; }
     | ifStmt                { $$ = $1; }
     | whileStmt             { $$ = $1; }
     | doWhileStmt           { $$ = $1; }
     | fCallStmt             { $$ = $1; }
     | fieldFn ';'           { $$ = $1; }
     | BREAK ';'             { $$ = makeBreakNode(); }
     | CONTINUE ';'          { $$ = makeContinueNode(); }
     | BRKP ';'              { $$ = makeBrkpNode(); }
     ;

fCallStmt : ID '(' argList ')' ';' { $$ = makeFnCallNode($1, $3); }
          | ID '(' ')' ';'         { $$ = makeFnCallNode($1, NULL); }

inputStmt : READ '(' identifier ')' ';'    { $$ = makeReadNode($3); }
          ;

outputStmt : WRITE '(' expr ')' ';' { $$ = makeWriteNode($3); }
           ;

initializeStmt : INITIALIZE '(' ')' ';' { $$ = makeInitializeNode(); }
              ;

freeStmt : FREE '(' ID ')' ';' { $$ = makeFreeNode($3); }
         | FREE '(' field ')' ';' { $$ = makeFreeNode($3); }
         ;

assignStmt : identifier '=' expr ';'        { $$ = makeOpNode("=", $1, $3); }
           ;

ifStmt : IF '(' expr ')' THEN stmtList ELSE stmtList ENDIF ';'  { $$ = makeIfElseNode($3, $6, $8); }
       | IF '(' expr ')' THEN stmtList ENDIF ';'           { $$ = makeIfNode($3, $6); }
       ;

whileStmt : WHILE '(' expr ')' DO stmtList ENDWHILE ';'      { $$ = makeWhileNode($3, $6); }
          ;

doWhileStmt : DO stmtList WHILE '(' expr ')' ';'      { $$ = makeDoWhileNode($2, $5); }
           ;

expr : expr '+' expr         { $$ = makeOpNode("+", $1, $3); }
     | expr '-' expr         { $$ = makeOpNode("-", $1, $3); }
     | expr '*' expr         { $$ = makeOpNode("*", $1, $3); }
     | expr '%' expr         { $$ = makeOpNode("%", $1, $3); }
     | expr '/' expr         { $$ = makeOpNode("/", $1, $3); }
     | expr '<' expr         { $$ = makeOpNode("<", $1, $3); }
     | expr '>' expr         { $$ = makeOpNode(">", $1, $3); }
     | expr LE expr          { $$ = makeOpNode("<=", $1, $3); }
     | expr GE expr          { $$ = makeOpNode(">=", $1, $3); }
     | expr EQ expr          { $$ = makeOpNode("==", $1, $3); }
     | expr NE expr          { $$ = makeOpNode("!=", $1, $3); }
     | '(' expr ')'          { $$ = $2; }
     | ALLOC '(' ')'         { $$ = makeAllocNode(); }
     | INITIALIZE '(' ')'    { $$ = makeInitializeNode(); }
     | FREE '(' ID ')'       { $$ = makeFreeNode($3); }
     | FREE '(' field ')'    { $$ = makeFreeNode($3); }
     | fCallStmt             { $$ = $1; }
     | NUM                   { $$ = $1; }
     | STRVAL                { $$ = $1; }
     | identifier            { $$ = $1; }
     | NULLVAL               { $$ = makeNullNode(); }
     | fieldFn               { $$ = $1; }
     | NEW '(' ID ')'        { $$ = makeNewNode($3); }
     ;

field : identifier '.' ID     { $$ = makeFieldNode($1, $3); }
      ;

fieldFn : identifier '.' ID '(' argList ')' { $$ = makeFieldFnNode($1, $3, $5); }
        | identifier '.' ID '(' ')'         { $$ = makeFieldFnNode($1, $3, NULL); }
        ; 

argList : argList ',' arg   { $$ = makeConnectorNode($1, $3); }
        | arg               { $$ = $1; }
        ;

arg : expr    { $$ = makeArgNode($1); }
    ;

identifier : ID    { $$ = $1; }
           | ID '[' index ']'   { $$ = makeArrayNode($1, $3); }
           | '&' ID             { $$ = makeAddressNode($2); }
           | ptr  %prec PTR     { $$ = $1; }
           | field              { $$ = $1; }
           ;

ptr : '*' ptr   { $$ = makePtrNode($2); }
    | '*' ID    { $$ = makePtrNode($2); }
    ;

index : expr    { $$ = $1; }

// --------------------------MAIN BLOCK-----------------------------------

mainBlock : INT MAIN '(' ')' '{' lDeclBlock fBody '}'   { $$ = makeMainNode($6, $7); }
          ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineno);
    exit(1);
}

int main() {
    yyin = fopen("io/input.txt", "r");

    ttInitialize();
    yyparse();
    ttPrint();
    printAST(root, "", 1);
    semantics(root);

    FILE* targetFile = fopen("output.xsm", "w");
    if (targetFile == NULL) {
        fprintf(stderr, "Could not open output.xsm for writing\n");
        return 1;
    }

    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "BRKP\n");
    fprintf(targetFile, "MOV SP, %d\n", getSP());
    fprintf(targetFile, "PUSH BP\n");
    fprintf(targetFile, "CALL M0\n");
    codeGen(root, targetFile);

    printAST(root, "", 1);
    printGST(gstRoot);

    fclose(targetFile);

    translateLabels(targetFile);

    return 0;
}