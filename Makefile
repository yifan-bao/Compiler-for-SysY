main:
	flex syntax.l
	bison -d syntax.y
	gcc -o syntax -g lex.yy.c syntax.tab.c node.c list.c vector.c symtable.c intercode2.c
clean:
	rm lex.yy.c syntax.tab.c syntax.tab.h 
	
