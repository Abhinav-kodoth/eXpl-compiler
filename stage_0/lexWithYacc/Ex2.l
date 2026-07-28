%{
#include "y.tab.h"
#include <string.h>
%}

%%

[a-zA-Z]+   { yylval.c = strdup(yytext); return WORD; }
"+"         { return PLUS; }
"*"         { return MUL; }
[\n]        { return NEWLINE; }
[ \t]       { /* skip whitespace */ }
.           { return *yytext; }

%%

int yywrap() { return 1; }