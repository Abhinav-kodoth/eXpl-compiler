%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tnode.h"      /* must come BEFORE y.tab.h */
    #include "y.tab.h"
%}

%%
[a-zA-Z]+   { printf("ID "); yylval = makeIdNode(yytext); return ID; }
[0-9]+      { printf("NUM ");yylval = makeNumNode(atoi(yytext)); return NUM; }
"="         { printf("= ");return '='; }
"+"         { printf("+ ");return '+'; }
"*"         { printf("- ");return '*'; }
";"         { return ';'; }
[ \t\n]     { /* skip whitespace */ }
.           { /* ignore anything else */ }
%%

int yywrap(void) {
    return 1;
}