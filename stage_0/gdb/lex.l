%{
  #include  "y.tab.h"
  #include "tree.h"
  void yyerror(char *);

%}


alpha   [a-zA-Z]

%%

{alpha}+  {yylval.p = createTree(yytext, NULL, NULL); return ID;}
"+"       {return PLUS;}
"-"       {return MINUS;}
"*"       {return MUL;}
"/"       {return DIV;}
[()]      {return *yytext;}
[ \t\n]           {}
.               {yyerror("invalid character\n");}


%%

int yywrap(void) {
  return 1;
}
