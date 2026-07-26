%{
#include<stdio.h>
int int_count = 0;
int float_count = 0;
%}

digit   [0-9]

%%
{digit}+\.{digit}+    { float_count++; }
{digit}+              { int_count++; }
.                     ;
\n                    ;
%%

int yywrap() { return 1; }

int main()
{
    yylex();
    printf("Integers : %d\n", int_count);
    printf("Floats   : %d\n", float_count);
    return 0;
}