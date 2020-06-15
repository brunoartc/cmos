#include "codegen.h"
#include "node.h"
#include "parser.h"

using namespace std;

extern NBlock *programBlock;
extern int yyparse();

extern int yylex();
extern FILE *yyin;

int main(int argc, char *argv[]) {
  yyparse();
  CodeGenContext context;
  context.generateCode(programBlock);
  context.runCode();
  fclose(yyin);
  return 0;
}
