%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "y.tab.h"
    #include "labelTranslation.h"

    int currPass = 1 ;
    int funcCurrAddress = -8 ;
    int currAddress = -8 ;
    FILE * op ;
    FILE * ip ;

    struct labelAddressTable * lat ;
    struct funcAddressNode * func = NULL ;
%}

%%

[F][0-9]+:\n        {   if(currPass == 1) func = addFunc(func, yytext, 2056 + 2*(funcCurrAddress)); }

[F][0-9]+\n         {
                        if(currPass == 2) {
                            int ans = searchFuncAddr(func,yytext);

                            if(ans == -1) {
                                perror(":Function label Not Found\n");
                            } else {
                                fprintf(op, "%d\n",ans);
                            }
                        } else {
                            funcCurrAddress++ ;
                            currAddress++ ;
                        }
                    }

[L][0-9]+:\n        {   if(currPass == 1) addLabel(lat, yytext, 2056 + 2*(currAddress)); }

[L][0-9]+\n         { 
                        if(currPass == 2) {
                            int ans = searchLabel(lat,yytext);

                            if(ans == -1) {
                                perror(":Label Not Found\n");
                            } else {
                                fprintf(op,"%d\n",ans);
                            }
                        } else {
                            currAddress++ ;
                            funcCurrAddress++ ;
                        }
                    }

"\n"                {
                        if(currPass == 1){
                            ++currAddress ;
                            ++funcCurrAddress ;
                        } else {
                            fprintf(op,"\n");
                        }
                    }

.                   {
                        if(currPass == 2) {
                            fprintf(op,"%s",yytext);
                        }
                    }

%%

int yyerror(char *s) {
    printf("yyerror: %s\n", s);
    exit(1);
}

int yywrap(void) {
	if(currPass == 1){
        printLabelTable(lat);
		fseek(ip, 0, SEEK_SET);
		++currPass;
		return 0;
	}else{
        return 1;
    }
}

int main(int argc, char **argv) {
    lat = (struct labelAddressTable *) malloc(sizeof(struct labelAddressTable));
    lat->head = NULL ;

    if(argc > 1) {
        ip = fopen(argv[1],"r");
        if(ip) yyin = ip ;
    }
    op = fopen("finaloutput.xsm","w");

    yylex();

    fclose(ip);
    fclose(op);

    return 0;
}