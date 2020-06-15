all: cmos

OBJS = lexer.o parser.o main.o codegen.o external.o

LLVMCONFIG = llvm-config-6.0
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -g
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -rdynamic
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf parser.cpp parser.h lexer.cpp lexer.h *.o cmos

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.h: parser.cpp

lexer.cpp: lexer.l parser.h
	flex -o $@ $^

lexer.h: lexer.cpp

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<

cmos: $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

TOBJS = lexer.o parser.o tests/test.o codegen.o external.o
	