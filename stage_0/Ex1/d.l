%{
#include<stdio.h>
%}

%%
"//".*                  { printf("Single-line comment: %s\n", yytext); }

"/*"([^*]|\*+[^*/])*\*+"/"   { printf("Multi-line comment: %s\n", yytext); }

.                        ;
\n                       ;
%%

int yywrap() { return 1; }

int main()
{
    yylex();
    return 0;
}