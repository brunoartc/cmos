%code requires {
  #include<string>
  #include<vector>
  #include "node.h"
}

%{
#include "node.h"
NBlock *programBlock;

extern int yylineno;
extern int yycolumn;
extern int yyleng;
extern int yylex();
void yyerror(const char *message);
%}

%output  "parser.cpp"
%defines "parser.h"

%union {
  Node *node;
  NBlock *block;
  NIdentifier *ident;
  NArray *array;
  std::string *string;
  int token;
}

%token-table
%token<token> CODE_START CODE_END
%token<string> VARID FUNID INTEGER STRING
%token<token> NEQ EQ LT GT LE GE
%token<token> LPR RPR L_BRACKETS R_BRACKETS L_BRACE R_BRACE
%token<token> ADD SUB MUL DIV
%token<token> ASSIGN COMMA SEMICOLL
%token<token> IF THEN ELSE WHILE BREAK FUNCTION RETURN

%left ADD SUB
%left MUL DIV

%start program

%type <block> program stmts
%type <array> array
%type <array> call_args

%type <node> ident assignment func_decl
%type <node> literals expr
%type <node> methodcall
%type <node> stmt
%type <node> arrid index
%type <node> math factor
%type <node> ifstmt loop cond
%type <node> return

%%

program : CODE_START CODE_END { programBlock = new NBlock(); }
        | CODE_START stmts CODE_END { programBlock = $2; }
        ;
  
stmts : stmt { $$ = new NBlock(); $$->statements.push_back($1); }
      | stmts stmt { $1->statements.push_back($2); }
      ;

stmt  : assignment SEMICOLL
      | func_decl
      | methodcall SEMICOLL
      | ifstmt
      | loop
      | return SEMICOLL
      ;

ifstmt : IF LPR cond RPR stmt { $$ = new NIf(*$3, *$5, *$5); }
       | IF LPR cond RPR L_BRACE stmts R_BRACE { $$ = new NIf(*$3, *$6, *$6); }
       | IF LPR cond RPR L_BRACE stmts R_BRACE ELSE L_BRACE stmts R_BRACE { $$ = new NIf(*$3, *$6, *$10); }
       ;

loop  : WHILE LPR cond RPR L_BRACE stmts R_BRACE { $$ = new NLoop(*$3, *$6); }
      ;


func_decl : FUNCTION ident LPR call_args RPR L_BRACE stmts R_BRACE 
			{ $$ = new NMethodDeclare(*$2, *$4, *$7); }


call_args : /*blank*/  { $$ = new NArray(); }
		  | factor { $$ = new NArray(); $$->data.push_back($1); }
		  | call_args COMMA factor  { $1->data.push_back($3); }
		  ;
              

methodcall : ident LPR call_args RPR { $$ = new NMethodCall(*$1, *$3);  }
            ;

return : RETURN factor{ $$ = new NReturn($2); }
        ;

expr  : factor
      | L_BRACKETS array R_BRACKETS { $$ = $2; }
      ;

assignment : ident ASSIGN expr { $$ = new NAssignment(*$1, *$3); }
            ;

factor : ident
       | literals
       | math
       | arrid
       | methodcall
       ;

array : factor { $$ = new NArray(); $$->data.push_back($1); } 
      | array COMMA factor { $1->data.push_back($3); }
      ;

cond  : factor LT factor { $$ = new NCond(3, *$1, *$3); }
      | factor GT factor { $$ = new NCond(5, *$1, *$3); } 
      | factor LE factor { $$ = new NCond(4, *$1, *$3); }
      | factor GE factor { $$ = new NCond(6, *$1, *$3); }
      | factor EQ factor { $$ = new NCond(1, *$1, *$3); }
      | factor NEQ factor { $$ = new NCond(2, *$1, *$3); }
      ;

math: factor ADD factor { $$ = new NMath(1, *$1, *$3); }
    | factor SUB factor { $$ = new NMath(2, *$1, *$3); }
    | factor MUL factor { $$ = new NMath(3, *$1, *$3); }
    | factor DIV factor { $$ = new NMath(4, *$1, *$3); }
    | LPR factor RPR {$$ = $2;};
    ;

arrid : ident L_BRACKETS index R_BRACKETS { $$ = new NArrid(*$1, *$3); };

literals  : INTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
          | STRING { $$ = new NString(*$1); delete $1; }
          ;

index : literals | ident;

ident : VARID { $$ = new NIdentifier(*$1); delete $1; }
      | FUNID { $$ = new NIdentifier(*$1); delete $1; }
      ;



%%

void yyerror(const char *s) { printf("parser_error: %s, line<%d>\n", s, yylineno); }

const char* get_token_name(int t) { return yytname[YYTRANSLATE(t)]; }