CXX=clang++
ANALYZER=scan-build -v
CFLAGS=-Wall -Wextra -Wall -Werror -pedantic -std=c++11
STDLIB=-stdlib=libc++
LIB=

SRC=./src
INC=./inc
OBJ=./obj
REP=./rep

INCLUDE=-I/usr/include/clang/3.5/include -I$(INC)

AFLAGS=--use-c++ `which $(CXX)` -o $(REP) --keep-going -maxloop 10 -enable-checker alpha.core.BoolAssignment -enable-checker alpha.core.CastSize -enable-checker alpha.core.CastToStruct -enable-checker alpha.core.PointerArithm -enable-checker alpha.core.PointerSub -enable-checker alpha.core.SizeofPtr -enable-checker alpha.cplusplus.NewDeleteLeaks -enable-checker alpha.cplusplus.VirtualCall -enable-checker alpha.deadcode.UnreachableCode -enable-checker alpha.security.ArrayBoundV2 -enable-checker alpha.security.ReturnPtrRange -enable-checker alpha.security.taint.TaintPropagation -enable-checker alpha.unix.Stream -enable-checker alpha.unix.cstring.BufferOverlap -enable-checker alpha.unix.cstring.NotNullTerminated -enable-checker alpha.unix.cstring.OutOfBounds -enable-checker security.FloatLoopCounter -enable-checker security.insecureAPI.rand -enable-checker security.insecureAPI.strcpy

JIT=jit
LEX=lex

OBJECTS= \
	$(OBJ)/token.o \
	$(OBJ)/scanner.o \
	$(OBJ)/ast.o \
	$(OBJ)/parser.o

all: $(OBJ) $(JIT) $(LEX)

objects: $(OBJ) $(OBJECTS)

$(LEX): $(OBJECTS) $(OBJ)/lexer.o
	$(CXX) $(STDLIB) $(LIB) -o $@ $+

$(JIT): $(OBJECTS) $(OBJ)/main.o
	$(CXX) $(STDLIB) $(LIB) -o $@ $+

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(STDLIB) -MMD $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ):
	mkdir -p $(OBJ)

check: $(OBJ) $(JIT) $(LEX)
	@echo "SCANNER TESTS:"
	bash ./tst/lex.sh ./lex

analyze_build:
	$(ANALYZER) $(AFLAGS) make

analyze_objects:
	$(ANALYZER) $(AFLAGS) make objects

-include $(OBJECTS:%.o=%.d)

clean:
	rm -rf $(REP)
	rm -rf $(OBJ)
	rm -rf $(JIT)
	rm -rf $(LEX)

.PHONY : clean
