#!/bin/bash
lex -o label_lex.c labelTranslation.l
gcc label_lex.c labelTranslation.c -o translater