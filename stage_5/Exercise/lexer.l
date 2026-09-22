%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "y.tab.h"
    #include "exptree.h"

    int yylineNum = 1;
%}

numbers     [0-9]+
letter      [a-zA-Z]
digit       [0-9]
id          {letter}({letter}|{digit})*

%%
            
"begin"     return BEG ;
"end"       return END ;
"return"    return RETURN ;
"Exit"      return EXIT_PR ;

"read"      return READ ; 
"write"     return WRITE ;

"tuple"     return TUPLE ;
"."         return DOT ;
"main"      return MAIN ;
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


"int"       return INT ;
"str"       return STR ;

"AND"       return AND ;
"&&"        return AND ;
"OR"        return OR ;
"||"        return OR ;
"NOT"       return NOT ;
"!"         return NOT ;

"decl"      return DECL ;
"enddecl"   return ENDDECL ;

"<"         return LT ;
"<="        return LE ;
"=="        return EQ ;
"!="        return NE ;
">"         return GT ;
">="        return GE ;

";"         return SEMICOLON ;

"&"         return AMPERSAND ;
"="         return ASSIGN ;
"+"         return PLUS ;
"-"         return MINUS ;
"*"         return STAR ;
"/"         return DIV ;

","         return *yytext;
[{}()\[\]]  return *yytext ;
"\n"        { yylineNum++ ;}
[ \t]     {}
{id}        {
                yylval.node = createTreeNode(0,NO_VAL,T_VOID,strdup(yytext),ID_NODE,NULL,NULL,NULL,NULL);
                return ID ;
            }

{numbers}   {   
                int num = atoi(yytext); 
                yylval.number  = num ;
                return NUM ;
            }

\"([^\"\\]|\\.)*\"  {
                        char * raw = yytext ;
                        char * s = strdup(raw + 1);
                        s[strlen(s) - 1] = '\0' ;
                        yylval.string = s;
                        return STR_LITERAL ;
                    }

.           {}

%%

int yywrap() {
    return 1;
}