DIRS = codegen tree translate

all: parser

parser: lex.yy.o lex.tl.o parse.tab.o tree.o codegen.o libfuncs.o utils.o translate_helpers.o gst.o lst.o semantics.o tt.o
	gcc -g -o parser lex.yy.o lex.tl.o parse.tab.o tree.o codegen.o libfuncs.o utils.o translate_helpers.o gst.o lst.o semantics.o tt.o
	make clean

semantics.o: semantics/semantics.c
	gcc -g -c semantics/semantics.c

tt.o: type_table/tt.c
	gcc -g -c type_table/tt.c

gst.o: symbol_table/gst.c
	gcc -g -c symbol_table/gst.c

lst.o: symbol_table/lst.c
	gcc -g -c symbol_table/lst.c

codegen.o: codegen/codegen.c
	gcc -g -c codegen/codegen.c

libfuncs.o: codegen/libfuncs.c
	gcc -g -c codegen/libfuncs.c

utils.o: codegen/utils.c
	gcc -g -c codegen/utils.c

translate_helpers.o: translate/translate_helpers.c
	gcc -g -c translate/translate_helpers.c

lex.tl.o: lex.tl.c
	gcc -g -c lex.tl.c

lex.yy.c: lex.l parse.tab.h
	flex lex.l

lex.tl.c: translate/translate.l
	flex translate/translate.l

parse.tab.c parse.tab.h: parse.y
	bison -d parse.y

lex.yy.o: lex.yy.c
	gcc -g -c lex.yy.c

parse.tab.o: parse.tab.c
	gcc -g -c parse.tab.c

tree.o: tree/tree.c
	gcc -g -c tree/tree.c

clean:
	rm *.o
	rm lex.yy.c
	rm lex.tl.c
	rm parse.tab.c
	rm parse.tab.h