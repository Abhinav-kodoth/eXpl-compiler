%{
#include <stdio.h>
#include <stdlib.h>
#include "tnode.h"
#include "codegen.h"

extern FILE *yyin;
int yylex(void);
int yyerror(char const *s);

static char *out_filename = "a.xsm";  /* default output, overridden by argv[2] */
%}

%token NUM PLUS END

%left PLUS

%%

start : E END {
                 printf("Prefix:  ");
                 printPrefix($1);
                 printf("\n");

                 printf("Postfix: ");
                 printPostfix($1);
                 printf("\n");

                 generate_code($1, out_filename);
                 printf("Wrote XEXE executable to %s\n", out_filename);

                 exit(0);
              }
      ;

E : E PLUS E  { $$ = makeOperatorNode('+', $1, $3); }
  | '(' E ')' { $$ = $2; }
  | NUM        { $$ = $1; }
  ;

%%

int yyerror(char const *s)
{
    printf("Error: %s\n", s);
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    if (argc > 1) {
        fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
    }
    if (argc > 2) {
        out_filename = argv[2];
    }
    yyparse();
    return 0;
}