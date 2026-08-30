yacc -d exptree.y
lex exptree.l
gcc lex.yy.c y.tab.c exptree.c symbolTable.c codeGen.c