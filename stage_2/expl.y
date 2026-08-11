%{
#include <stdio.h>
#include <stdlib.h>
#include "tnode.h"

extern FILE *yyin;
int yylex(void);
int yyerror(char const *s);

tnode *astRoot;
%}

%token BEGIN_T END_T READ WRITE ID NUM
%token PLUS MINUS MUL DIV ASSIGN

%left PLUS MINUS
%left MUL DIV

%%

Program : BEGIN_T Slist END_T ';' { astRoot = $2; }
        | BEGIN_T END_T ';'        { astRoot = NULL; }
        ;

Slist : Slist Stmt { $$ = createTree(0, CONNECTOR_T, NULL, $1, $2); }
      | Stmt         { $$ = $1; }
      ;

Stmt : InputStmt
     | OutputStmt
     | AsgStmt
     ;

InputStmt : READ '(' ID ')' ';' { $$ = createTree(0, READ_T, NULL, $3, NULL); }
          ;

OutputStmt : WRITE '(' E ')' ';' { $$ = createTree(0, WRITE_T, NULL, $3, NULL); }
           ;

AsgStmt : ID ASSIGN E ';' { $$ = createTree(0, ASSIGN_T, NULL, $1, $3); }
        ;

E : E PLUS E  { $$ = createTree(0, PLUS_T, NULL, $1, $3); }
  | E MINUS E { $$ = createTree(0, MINUS_T, NULL, $1, $3); }
  | E MUL E   { $$ = createTree(0, MUL_T, NULL, $1, $3); }
  | E DIV E   { $$ = createTree(0, DIV_T, NULL, $1, $3); }
  | '(' E ')'  { $$ = $2; }
  | ID          { $$ = $1; }
  | NUM         { $$ = $1; }
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
    printAST(astRoot, 0);
    return 0;
}