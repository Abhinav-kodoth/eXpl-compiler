%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tnode.h"

    extern FILE *yyin;
    int yylex(void);
    void yyerror(const char *s);

    tnode *astRoot = NULL;
%}

%token NUM ID
%left '+'
%left '*'

%%
S : ID '=' E ';' { $$ = makeAssignNode($1, $3); astRoot = $$; }
  ;

E : E '+' E { $$ = makeOperatorNode('+', $1, $3); }
  | E '*' E { $$ = makeOperatorNode('*', $1, $3); }
  | NUM     { $$ = $1; }
  | ID      { $$ = $1; }
  ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) { perror(argv[1]); return 1; }
        yyin = fp;
    }
    if (yyparse() == 0) {
        printf("=== AST ===\n");
        printAST(astRoot, 0);
    }
    return 0;
}