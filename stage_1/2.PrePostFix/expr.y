%{
#include <stdio.h>
#include <stdlib.h>
#include "tnode.h"

extern FILE *yyin;
int yylex(void);
int yyerror(char const *s);
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

                 exit(1);
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
    yyparse();
    return 0;
}