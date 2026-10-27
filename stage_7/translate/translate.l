%{
    #include "translate/translate.h"
    void yyerror(const char*);

    llnode* head = NULL;
    int lineNumber = 0;
    int passcount = 1;

    FILE* tempfile;
%}

%option prefix="tl"

%%

[a-zA-Z][a-zA-Z0-9]*[:][ \t\n]+ {
    if(passcount == 1) {
        for(int i = 0; i < strlen(tltext); i++) {
            if(tltext[i] == ':') {
                tltext[i] = '\0';
                break;
            }
        }
        int address = 2056 + 2*(lineNumber - 8);
        head = insert(tltext, address, head);
    }
}

[a-zA-Z][a-zA-Z0-9]* {
    if(passcount == 2) {
        int address = getAddress(tltext, head);
        if(address != -1) {
            fprintf(tempfile, "%d", address);
        }
        else{
            fprintf(tempfile, "%s", tltext);
        }
    }
}

"\n" { 
    if(passcount == 1) {
        lineNumber++;
    }
    else{
        fprintf(tempfile, "\n");
    }
}

.  { 
    if(passcount == 2) {
        fprintf(tempfile, "%s", tltext);
    }
 }

%%

int tlwrap() {
    if(passcount == 1) {
        passcount++;
        lineNumber = 0;
        rewind(tlin);

        tempfile = fopen("output_temp.xsm", "w");\

        return 0; 
    }

    fclose(tempfile);

    remove("output.xsm");
    rename("output_temp.xsm", "output.xsm");

    return 1;
}

void translateLabels(FILE* inputFile) {
    tlin = fopen("output.xsm", "r");
    // printf("Translating Labels...\n");
    while(tllex() != 0);
}