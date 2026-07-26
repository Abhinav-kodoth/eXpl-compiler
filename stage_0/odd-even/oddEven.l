%option noyywrap

%{
#include <stdlib.h>
#include <stdio.h>
%}

number [0-9]+

%%

{number} {
    int num = atoi(yytext);

    if (num % 2 == 0)
        printf("Found: %d (Even)\n", num);
    else
        printf("Found: %d (Odd)\n", num);
}

%%

int main()
{
    yylex();
    return 0;
}