%{
#include<stdio.h>
#include<ctype.h>
%}

%%
[Aa][a-zA-Z][a-zA-Z][a-zA-Z]+   ;   /* skip longer words starting with A */
[Aa][a-zA-Z][a-zA-Z]            {
                                   int i;
                                   for(i = 0; i < yyleng; i++)
                                       putchar(toupper(yytext[i]));
                                   printf("\n");
                                 }
[a-zA-Z]+                       ;
.                                ;
\n                               ;
%%

int yywrap() { return 1; }

int main()
{
    yylex();
    return 0;
}