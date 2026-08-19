%{
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern FILE *yyin;
int yylex(void);
int yyerror(char const *s);
node *astRoot;
%}
%token BEGIN_T END_T READ WRITE ID NUM
%token PLUS MINUS MUL DIV ASSIGN
%token IF THEN ELSE ENDIF WHILE DO ENDWHILE
%token LT GT LE GE NE EQ
%token BREAK CONTINUE BRKP
%left PLUS MINUS
%left MUL DIV
%nonassoc LT GT LE GE NE EQ
%%
Program : BEGIN_T Slist END_T ';' { astRoot = $2; }
        | BEGIN_T END_T ';'        { astRoot = makeEmptyNode(); }
        ;

Slist : Slist Stmt { $$ = makeConnectorNode($1, $2); }
      | Stmt         { $$ = $1; }
      ;

Stmt : InputStmt
     | OutputStmt
     | AsgStmt
     | IfStmt
     | WhileStmt
     | DoWhileStmt
     | BreakStmt
     | ContinueStmt
     | BrkpStmt
     ;

InputStmt : READ '(' ID ')' ';' { $$ = makeReadNode($3); }
          ;

OutputStmt : WRITE '(' E ')' ';' { $$ = makeWriteNode($3); }
           ;

AsgStmt : ID ASSIGN E ';' { $$ = makeAssignNode($1, $3); }
        ;

IfStmt : IF '(' E ')' THEN Slist ELSE Slist ENDIF ';'
             { $$ = makeIfElseNode($3, $6, $8); }
       | IF '(' E ')' THEN Slist ENDIF ';'
             { $$ = makeIfNode($3, $6); }
       ;

WhileStmt : WHILE '(' E ')' DO Slist ENDWHILE ';'
             { $$ = makeWhileNode($3, $6); }
          ;

DoWhileStmt : DO Slist WHILE '(' E ')' ';'
             { $$ = makeDoWhileNode($2, $5); }
            ;

BreakStmt : BREAK ';'    { $$ = makeBreakNode(); }
          ;

ContinueStmt : CONTINUE ';' { $$ = makeContinueNode(); }
             ;

BrkpStmt : BRKP ';'      { $$ = makeBrkpNode(); }
         ;

E : E PLUS E  { $$ = makeOpNode("+",  $1, $3); }
  | E MINUS E { $$ = makeOpNode("-",  $1, $3); }
  | E MUL E   { $$ = makeOpNode("*",  $1, $3); }
  | E DIV E   { $$ = makeOpNode("/",  $1, $3); }
  | E LT E    { $$ = makeOpNode("<",  $1, $3); }
  | E GT E    { $$ = makeOpNode(">",  $1, $3); }
  | E LE E    { $$ = makeOpNode("<=", $1, $3); }
  | E GE E    { $$ = makeOpNode(">=", $1, $3); }
  | E NE E    { $$ = makeOpNode("!=", $1, $3); }
  | E EQ E    { $$ = makeOpNode("==", $1, $3); }
  | '(' E ')' { $$ = $2; }
  | ID        { $$ = $1; }
  | NUM       { $$ = $1; }
  ;
%%
int yyerror(char const *s) {
    printf("Error: %s\n", s);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    yyparse();
    printTree(astRoot, 0);
    return 0;
}