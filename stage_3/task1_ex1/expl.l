%{
#include "tree.h"
#include "y.tab.h"
%}
%%
"begin"     { return BEGIN_T; }
"end"       { return END_T; }
"read"      { return READ; }
"write"     { return WRITE; }
"if"        { return IF; }
"then"      { return THEN; }
"else"      { return ELSE; }
"endif"     { return ENDIF; }
"while"     { return WHILE; }
"do"        { return DO; }
"endwhile"  { return ENDWHILE; }
"break"     { return BREAK; }
"continue"  { return CONTINUE; }
"brkp"      { return BRKP; }
[a-z]       { yylval = makeLeafIdNode(yytext); return ID; }
[0-9]+      { yylval = makeLeafNumNode(atoi(yytext)); return NUM; }
"<="        { return LE; }
">="        { return GE; }
"!="        { return NE; }
"=="        { return EQ; }
"<"         { return LT; }
">"         { return GT; }
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MUL; }
"/"         { return DIV; }
"="         { return ASSIGN; }
"("         { return '('; }
")"         { return ')'; }
";"         { return ';'; }
[ \t\n]     { }
.           { }
%%
int yywrap() { return 1; }