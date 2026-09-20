%{
    #include <stdio.h>
%}


number [0-9]+
op [+|-|/|*|%]

%%

{number} {printf(" number");}
{op} {printf(" op");}

%%

int yywrap() {
    return 1;
}

int main(){
    yylex();
    return 1;
}