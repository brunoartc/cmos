%{
#include "parser.h"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

/* location */
extern int yylineno;
int yycolumn = 1;
void add_column(int yyleng);

/* error message */
void lexer_error(const char *message);
%}

%option noyywrap nodefault yylineno
%option outfile="lexer.cpp" header-file="lexer.h"

digit      [0-9]
letter     [a-zA-Z]
ws         [ \t]
var_id        ("$")("_"|{letter})("_"|{letter}|{digit})*
func_id        ("_"|{letter})("_"|{letter}|{digit})*
sign       [-+]
comment    "//".*
string     \"[^\"\n]*\"
integer    (0|([1-9]{digit}*))

%%

[\n]        { yycolumn = 1; }
{ws}+       { add_column(yyleng); }
{comment}   ;
"manifesto"     { add_column(yyleng); return TOKEN(CODE_START); }
"permuta"         { add_column(yyleng); return TOKEN(SEMICOLL); }
"preludio morfema"         { add_column(yyleng); return TOKEN(L_BRACE); }
"morfema epilogo"         { add_column(yyleng); return TOKEN(R_BRACE); }
"comensurado"         { add_column(yyleng); return TOKEN(ASSIGN); }
","         { add_column(yyleng); return TOKEN(COMMA); }


"apenso"         { add_column(yyleng); return TOKEN(ADD); }
"merma"         { add_column(yyleng); return TOKEN(SUB); }
"amiudado"         { add_column(yyleng); return TOKEN(MUL); }
"div"         { add_column(yyleng); return TOKEN(DIV); }

"par"        { add_column(yyleng); return TOKEN(EQ); }
"despar"        { add_column(yyleng); return TOKEN(NEQ); }
"infero"         { add_column(yyleng); return TOKEN(LT); }
"infepar"        { add_column(yyleng); return TOKEN(LE); }
"mor"         { add_column(yyleng); return TOKEN(GT); }
"morpar"        { add_column(yyleng); return TOKEN(GE); }



"na conjuntura de"        { add_column(yyleng); return TOKEN(IF); }
"caso adverso"      { add_column(yyleng); return TOKEN(ELSE); }
"durante o atimo de"     { add_column(yyleng); return TOKEN(WHILE); }
"tramite"  { add_column(yyleng); return TOKEN(FUNCTION); }
"("         { add_column(yyleng); return TOKEN(LPR); }
")"         { add_column(yyleng); return TOKEN(RPR); }
"epigrafe"         { add_column(yyleng); return TOKEN(L_BRACKETS); }
"epigrafe epilogo"         { add_column(yyleng); return TOKEN(R_BRACKETS); }
"valer"    { add_column(yyleng); return TOKEN(RETURN); }
"manifesto peroracao"        { add_column(yyleng); return TOKEN(CODE_END); }

{func_id}        { add_column(yyleng); SAVE_TOKEN; return FUNID; }
{var_id}        { add_column(yyleng); SAVE_TOKEN; return VARID; }
{integer}   { add_column(yyleng); SAVE_TOKEN; return INTEGER; }
{string}    { add_column(yyleng); SAVE_TOKEN; return STRING; }



.           { lexer_error("unknown literal"); yyterminate(); }

%%

void lexer_error(const char* message){
  printf("\033[0;31mlexer_error: %s, loc<%d:%d>\n", message, yylineno, yycolumn);
}

void add_column(int yyleng) {
  yycolumn += yyleng;
}