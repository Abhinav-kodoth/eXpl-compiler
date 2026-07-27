%{
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
int valid = 1;
%}
%token LETTER DIGIT
%%
start : var '\n'   {
                      if(valid)
                          printf("Valid variable\n");
                      else
                          printf("Invalid variable\n");
                      exit(1);
                    }
      | error '\n' {
                      printf("Invalid variable\n");
                      exit(1);
                    }
      ;

var : LETTER rest
    ;

rest : rest LETTER
     | rest DIGIT
     |
     ;
%%
yyerror(char const *s)
{
    valid = 0;
}

yylex()
{
    int c = getchar();
    if(c == '\n')
        return c;
    if(isalpha(c))
        return LETTER;
    if(isdigit(c))
        return DIGIT;
    valid = 0;
    return c;
}

main()
{
    yyparse();
    return 1;
}