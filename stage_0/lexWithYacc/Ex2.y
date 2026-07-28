%{
#include <stdio.h>
#include <string.h>
int yyerror(char const *s);
%}

%union {
    char *c;
}

%token <c> WORD
%token PLUS MUL NEWLINE
%type <c> expr

%left PLUS
%left MUL

%%

start : expr NEWLINE { printf("\n"); exit(1); }
      ;

expr : expr PLUS expr { printf("%s ", "+"); }
     | expr MUL expr  { printf("%s ", "*"); }
     | WORD             { printf("%s ", $1); }
     ;

%%

int yyerror(char const *s)
{
    printf("Error: %s\n", s);
    return 0;
}

int main()
{
    yyparse();
    return 0;
}