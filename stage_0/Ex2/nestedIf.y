%{
#include<stdio.h>
#include<stdlib.h>

int level = 0;   /* tracks current nesting depth */
%}

%token IF END

%%

start : stmt_list '\n'  {printf("\nDone\n"); exit(1);}
      ;

stmt_list : stmt_list stmt
          |
          ;

stmt : IF          { level++; printf("IF at nesting level %d\n", level); }
       stmt_list
       END         { printf("END of nesting level %d\n", level); level--; }
     ;

%%

yyerror(char const *s)
{
    printf("Error: %s\n", s);
}

yylex()
{
    char buf[10];
    int i = 0, c;

    c = getchar();

    while(c == ' ')          /* skip whitespace */
        c = getchar();

    if(c == '\n')
        return c;

    if(isalpha(c))
    {
        while(isalpha(c))
        {
            buf[i++] = c;
            c = getchar();
        }
        buf[i] = '\0';
        ungetc(c, stdin);      /* push back the non-alpha char we over-read */

        if(strcmp(buf,"if") == 0)
            return IF;
        else if(strcmp(buf,"end") == 0)
            return END;
    }
    return c;
}

main()
{
    yyparse();
    return 1;
}