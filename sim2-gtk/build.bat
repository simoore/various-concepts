@PATH=c:\gtk\bin;%PATH%
@for /F "usebackq tokens=*" %%i in (`pkg-config --cflags --libs gtk+-3.0`) do ^
gcc -o bin/sim2 src/main.c src/sim.c src/compiler.c src/program.c src/lexer.c %%i