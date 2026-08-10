%{
#include "y.tab.h"
#include "tnode.h"
%}

%%
"begin"     { return BEGIN_T; }
"end"       { return END_T; }
"read"      { return READ; }
"write"     { return WRITE; }
[a-z]       { yylval = createTree(0, ID_T, yytext, NULL, NULL); return ID; }
[0-9]+      { yylval = createTree(atoi(yytext), NUM_T, NULL, NULL, NULL); return NUM; }
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