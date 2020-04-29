
%{
#include <stdio.h>

int yylex();
int yyerror(char *s);

%}

%token STRING NUM OTHER SEMICOLON STATEWHILE STATEIF STATEELSE BCHAVE ACHAVE MAIS MENOS MENOR IGUAL INT DCHAVE CCHAVE

%%

statement:
   | STATEIF paren_expr statement STATEELSE statement
   | STATEWHILE paren_expr statement
   | ACHAVE statement BCHAVE
   | expr SEMICOLON
   | SEMICOLON
   ;

paren_expr
   : CCHAVE expr DCHAVE
   ;

expr
   : test
   | STRING IGUAL expr
   ;

test
   : sum
   | sum MENOR sum
   ;

sum
   : term
   | sum MAIS term
   | sum MENOS term
   ;

term
   : STRING
   | INT
   | paren_expr
   ;
   

%%

int yyerror(char *s)
{
	printf("Syntax Error on line %s\n", s);
	return 0;
}

int main()
{
	extern int yydebug;
	yydebug = 1;
    yyparse();
    return 0;
}
