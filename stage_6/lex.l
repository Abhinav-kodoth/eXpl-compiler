%{
    #include "parse.tab.h"
    #include "tree/tree.h"
    void yyerror(const char*);
%}

%option yylineno

digit [0-9]

%%

{digit}+ { yylval.p = makeLeafNumNode(atoi(yytext)); return NUM; }
\".*\"   {   
            yylval.p = makeLeafStrNode(strdup(yytext));
            return STRVAL; 
         }
"begin"  { return START; }
"end"    { return END; }
"read"   { return READ; }
"write"  { return WRITE; }
"if"     { return IF; }
"then"   { return THEN; }
"else"   { return ELSE; }
"endif"  { return ENDIF; }
"while"  { return WHILE; }
"do"     { return DO; }
"endwhile"   { return ENDWHILE; }
"break"      { return BREAK; }
"continue"   { return CONTINUE; }
"breakpoint" { return BRKP; }
"decl"       { return DECL; }
"enddecl"    { return ENDDECL; }
"int"     { return INT; }
"str"     { return STR; }
"null"    { return NULLVAL; }
"return"  { return RETURN; }
"main"    { return MAIN; }
"type"    { return TYPE; }
"endtype" { return ENDTYPE; }
"initialize" { return INITIALIZE; }
"alloc"   { return ALLOC; }
"free"    { return FREE; }
[a-zA-Z][a-zA-Z0-9]*    { yylval.p = makeLeafIdNode(yytext); return ID; }
"."      { return '.'; }
"+"      { return '+'; }
"-"      { return '-'; }
"*"      { return '*'; }
"/"      { return '/'; }
"%"      { return '%'; }
"&"      { return '&'; }
">"      { return '>'; }
"<"      { return '<'; }
">="     { return GE; }
"<="     { return LE; }
"=="     { return EQ; }
"!="     { return NE; }
"="      { return '='; }
";"      { return ';'; }
"("      { return '('; }
")"      { return ')'; }
"["      { return '['; }
"]"      { return ']'; }
"{"      { return '{'; }
"}"      { return '}'; }
","      { return ','; }
[ \t\n]  { /* ignore whitespace */ }
.        { yyerror("Invalid character\n"); }

%%

int yywrap() {
    return 1;
}
