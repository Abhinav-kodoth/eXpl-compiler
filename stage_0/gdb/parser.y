%{
  //#define YYSTYPE tnode*
  #include <stdio.h>
  #include <stdlib.h>
  #include "tree.h"
  #include "tree.c"
  extern struct tnode* idptr;
  int yylex(void);
  extern FILE *yyin;
  extern char* yytext;
%}

%union{
  struct tnode* p;
}

%token <p> ID

%type <p> expr

%left PLUS MINUS
%left MUL DIV
%%


Program :  expr    {infixtoprefix($1);printf("\n");free($1);}
        ;

expr :  expr PLUS expr  {$$ = createTree("+",$1, $3);}
     | expr MINUS expr  {$$ = createTree("-",$1, $3);}
     | expr MUL expr  {$$ = createTree("*",$1, $3);}
     | expr DIV expr  {$$ = createTree("/",$1, $3);}
     | '(' expr ')'  {$$ = $2;}
     | ID      {$$=$1;}
     ;

%%

void yyerror(char const *s)
{
    printf("yyerror '%s' and '%s'",s,yytext);
}


int main(int argc, char*argv[]) {
  if (argc > 1) {
      FILE *fp = fopen(argv[1],"r");
      if (fp) {
        yyin = fp;
      }
  }
  yyparse();
  return 0;
}
