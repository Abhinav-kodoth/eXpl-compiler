%{
 #include<stdio.h>
 char *file1;
%}

%%

[0-9]+  printf("number");

%%

int yywrap()
{
    if(strcmp(file1, "console") == 0)
    {
        yyin = fopen("input_file.l", "r");
        file1 = "input_file.l";
        return 0;
    }
    return 1;
}

int main()
{
    file1 = "console";
    yylex();
    return 0;
}