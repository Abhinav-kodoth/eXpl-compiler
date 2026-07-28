%{
#include <stdio.h>
int yyerror(char const *s);
%}

%union {
    char c;
}

%token <c> LETTER PLUS MUL
%token NEWLINE
%type <c> expr

%left PLUS
%left MUL

%%

start : expr NEWLINE { printf("\n"); exit(1); }
      ;

expr : expr PLUS expr { printf("%c", $2); }
     | expr MUL expr  { printf("%c", $2); }
     | LETTER          { printf("%c", $1); }
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