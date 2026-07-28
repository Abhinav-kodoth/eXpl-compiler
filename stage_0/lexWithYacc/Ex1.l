%{
#include "y.tab.h"
%}

%%

[a-zA-Z]    { yylval.c = *yytext; return LETTER; }
"+"         { yylval.c = *yytext; return PLUS; }
"*"         { yylval.c = *yytext; return MUL; }
[\n]        { return NEWLINE; }
[ \t]       { /* skip whitespace */ }
.           { return *yytext; }

%%

int yywrap() { return 1; }