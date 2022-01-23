SRC = src/main.c src/sim.c src/compiler.c src/program.c src/lexer.c
TEST_SRC = src/test-compiler.c src/compiler.c src/lexer.c src/program.c

all:
	mkdir -p bin
	gcc -o bin/sim2 $(SRC) `pkg-config --cflags --libs gtk+-3.0`
#gcc -o bin/test-compiler $(TEST_SRC) `pkg-config --cflags --libs gtk+-3.0`

run:
	bin/sim2
#bin/test-compiler
