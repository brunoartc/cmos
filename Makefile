default:
	clear
	bison -dv -t portold.y 
	flex -d -l portold.l
	gcc -o portold portold.tab.c lex.yy.c -lfl
