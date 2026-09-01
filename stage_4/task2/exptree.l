%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "y.tab.h"
    #include "exptree.h"

%}

numbers     [0-9]+
letter      [a-zA-Z]
digit       [0-9]
id          {letter}({letter}|{digit})*

%%
            
"begin"     return BEG ;
"end"       return END ;
"Exit"      return EXIT_PR ;

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

"int"       return INT ;
"str"       return STR ;

"decl"      return DECL ;
"enddecl"   return ENDDECL ;

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
"*"         return STAR ;
"/"         return DIV ;
"%"         return MOD ;

"&"         return AMPERSAND ;
","         return *yytext;
[()\[\]]    return *yytext ;
[ \t\n]     {}
{id}        {
                yylval.node = createTreeNode(NO_VAL,NO_TYPE,strdup(yytext),ID_NODE,NULL,NULL,NULL);
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