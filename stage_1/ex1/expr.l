%{
#include "tnode.h"
#include "y.tab.h"
%}

%%
[0-9]+      { yylval = makeLeafNode(atoi(yytext)); return NUM; }
"+"         { return PLUS; }
"("         { return '('; }
")"         { return ')'; }
[\n]        { return END; }
[ \t]       { }
.           { }
%%

int yywrap() { return 1; }