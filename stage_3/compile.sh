#!/bin/bash
lex -o exptree_lex.c exptree.l
yacc -d exptree.y
gcc exptree_lex.c y.tab.c exptree.c -o exptree
