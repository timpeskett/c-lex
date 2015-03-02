CC = gcc
CFLAGS = -g -ansi -pedantic -Wall -Werror -c 
LDFLAGS = 
OBJ = lexer.o Symbol.o regex.o nfa.o lextest.o symtable.o linklist.o

lextest : $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o lextest

lextest.o : lextest.c Symbol.h lexer.h
	$(CC) $(CFLAGS) lextest.c -o lextest.o

lexer.o : lexer.h lexer.c global.h nfa.h symtable.h Symbol.h
	$(CC) $(CFLAGS) lexer.c -o lexer.o

Symbol.o : Symbol.h Symbol.c global.h
	$(CC) $(CFLAGS) Symbol.c -o Symbol.o

regex.o : regex.h regex.c nfa.h global.h
	$(CC) $(CFLAGS) regex.c -o regex.o

nfa.o : nfa.h nfa.c global.h linklist.h
	$(CC) $(CFLAGS) nfa.c -o nfa.o

symtable.o : symtable.h symtable.c Symbol.h linklist.h
	$(CC) $(CFLAGS) symtable.c -o symtable.o

linklist.o : linklist.h linklist.c
	$(CC) $(CFLAGS) linklist.c -o linklist.o

clean :
	rm -f $(OBJ) lextest
