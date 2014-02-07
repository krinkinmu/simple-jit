CXX=clang++
CFLAGS=-Wall -Wextra -Wall -Werror -pedantic -std=c++11
STDLIB=

SRC=./src
INC=./inc
OBJ=./obj

JIT=jit
LEX=lex

OBJECTS= \
	$(OBJ)/token.o \
	$(OBJ)/scanner.o

all: $(OBJ) $(JIT) $(LEX)

$(LEX): $(OBJECTS) $(OBJ)/lexer.o
	$(CXX) -o $@ $+

$(JIT): $(OBJECTS) $(OBJ)/main.o
	$(CXX) -o $@ $+

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) -MMD $(CFLAGS) -I$(INC) -c $< -o $@

$(OBJ):
	mkdir -p $(OBJ)

check: $(OBJ) $(JIT) $(LEX)
	@echo "SCANNER TESTS:"
	bash ./tst/lex.sh ./lex

-include $(OBJECTS:%.o=%.d)

clean:
	rm -rf $(OBJ)
	rm -rf $(JIT)
	rm -rf $(LEX)

.PHONY : clean
