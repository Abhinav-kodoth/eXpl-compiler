%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "y.tab.h"
    #include "exptree.h"

    int number;
%}

numbers     [0-9]+
id          [a-zA-Z]

%%

{numbers}    {   
                number = atoi(yytext); 
                yylval.node = createTreeNode(number,INT_TYPE,NULL,NUM_NODE,NULL,NULL);
                return NUM ;
            }
            
{id}        {
                yylval.node = createTreeNode(NO_VAL,INT_TYPE,strdup(yytext),ID_NODE,NULL,NULL);
                return ID ;
            }
"begin"     return BEG ;
"end"       return END ;
"read"      return READ ; 
"write"     return WRITE ;
"if"        return IF ;
"then"      return THEN ;
"else"      return ELSE ;
"while"     return WHILE ;
"continue"  return CONTINUE ;
"break"     return BREAK ;
"repeat"    return REPEAT ;
"until"     return UNTIL ;
"endif"     return ENDIF ;
"endwhile"  return ENDWHILE ;
"do"        return DO ;
"<"         return LT ;
"<="        return LE ;
"=="        return EQ ;
"!="        return NE ;
">"         return GT ;
">="        return GE ;
";"         return SEMICOLON ;
"="         return ASSIGN ;
"+"         return PLUS ;
"-"         return MINUS ;
"*"         return MUL ;
"/"         return DIV ;
[ \t]       {}
[()]        return *yytext ;
[\n]        {}
.           {}

%%

int yywrap() {
    return 1;
}