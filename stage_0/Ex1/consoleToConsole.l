%{
 #include<stdio.h>
 char *file1;
%}

%%

[0-9]+  printf("number");

%%

int yywrap()
{
    if(count == 0)
    {
        count++;
        yyin = stdin;
        return 0;
    }
    return 1;
}

int main()
{
    yylex();
    return 0;
}