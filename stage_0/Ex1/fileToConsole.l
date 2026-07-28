%{
 #include<stdio.h>
 char *file1;
%}

%%

[0-9]+  printf("number");

%%

int yywrap()
{
    if(strcmp(file1, "input_file.l") == 0)
    {
        yyin = stdin;
        file1 = "console";
        return 0;
    }
    return 1;
}

int main()
{
    file1 = "input_file.l";
    yyin = fopen("input_file.l", "r");
    yylex();
    return 0;
}