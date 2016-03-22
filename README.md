# c-lex

c-lex is a lexical analyser for C, written in C. It is written completely from
scratch, with its own NFA/DFA implementation. I created c-lex in early 2014
after having read some of the infamous Dragon Book.

## Overview

The most bulk of the program is the DFA/NFA implementation. This isn't a
trivial task in C, because of the lack of memory management. The NFA
implementation performs a Thompson construction on a given regular expression
and then converts the resulting NFA into a DFA. The regular expression is
parsed using a simple recursive descent parser. The C lexer is then built on
top of this DFA/NFA implementation with the regular expressions for parsing C
built in to the program.

#### lextest.c/lextest.h

This is provides a simple entry point into the program. It just creates the
symbol table and it calls the lexical analyser (in lexer.c) to do its thing.
It also defines some handy strings to make the output human-readable.

#### lexer.c/lexer.h

This is the module that performs the file I/O and defines the C regular
expressions. In theory, this module and the symtable module could be changed
to build a lexical analyser for a different language.

#### nfa.c/nfa.h

Contains the NFA/DFA implementation. The Dfa and Nfa abstract types are defined
here. The code for performing the Thompson construction and the NFA to DFA
conversion is in the module.

For performing the Thompson construction, there are several important
functions:

```C
Nfa nfaCreateEmpty( void );
int nfaAddEmptyState( Nfa nfa, Bool isStart, Bool isFinish );
void nfaAddTransition( Nfa nfa, int from, int to, int symbol, Bool isAll );

void nfaSetStart( Nfa nfa, int state, Bool isStart );
void nfaSetFinish( Nfa nfa, int state, Bool isFinish );

/* Functions will return null if operation is not possible */
Nfa nfaConcat( Nfa n1, Nfa n2 );
Nfa nfaKleene( Nfa n1 );
Nfa nfaAlternate( Nfa n1, Nfa n2 );

/* These functions have the same effect but they do the operation in place instead of creating a new nfa */
void nfaConcatInPlace( Nfa dest, Nfa src );
void nfaKleeneInPlace( Nfa n1 );
void nfaAlternateInPlace( Nfa dest, Nfa src );
```

The Thompson construction begins by creating an empty NFA, and then using
nfaConcat, nfaKleene, and nfaAlternate functions to build on this original
NFA.

The resulting NFA can then be converted to a DFA using

```C
Dfa nfaToDfa( Nfa nfa);
```

and then the Dfa can be simulated using 

```C
Bool nfaSimulateDfa( Dfa dfa, char *string );
```

Note that the *nfa* prefix is just for namespacing. The implementation is not
optimised, and so may be slow on bigger files or slower machines. I am always
more concerned with writing correct code than writing fast code.

#### regex.c/regex.h

This is the recursive descent parser that creates an NFA from a regular
expression. It is a simple recursive descent parser much in the same vein as
those presented in the Dragon book.

#### symtable.c/symtable.h

This is a symbol table that contains all of the keywords of the language. It
allows the code to differentiate between identifiers and keywords in the input.

#### Symbol.c/Symbol.h

Defines a data structure to represent a symbol in the language. A symbol
consists of a token (e.g INTCONST) and a lexeme (e.g 42). It also defines two
functions for creating and destroying symbols. These days I tend to either
treat data structures as either entirely abstract or entirely transparent,
whereas this symbol structure is sort of a mix of both.

#### linklist.c/linklist.h

A simple linked list implementation. By this time I had written many linked
list implementations and so I was quite familiar with them. Nothing really
special here.

#### global.h

Contains some basic C definitions that are missing from the standard. Notably,
a boolean type is declared here.

## Example

The following C file is a simple hello world program. Note that it does not
have any #include statements, as the lexical analyser assumes that the file
has been preprocessed beforehand.

```C
int main(void) {
    printf("Hello World");

    return 0;
}
```

Below is the output from running `./lextest helloworld.c`. Notice how it
associates a token to all of the symbols in the file.

The error on the file line occurs when encountering EOF. This is a known bug.

```
Symbol found! Token: WHITESPACE -- Lexeme: 

Symbol found! Token: INT -- Lexeme: int
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: ID -- Lexeme: main
Symbol found! Token: LPAREN -- Lexeme: (
Symbol found! Token: VOID -- Lexeme: void
Symbol found! Token: RPAREN -- Lexeme: )
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: LBRACE -- Lexeme: {
Symbol found! Token: WHITESPACE -- Lexeme: 

Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: ID -- Lexeme: printf
Symbol found! Token: LPAREN -- Lexeme: (
Symbol found! Token: STRINGLIT -- Lexeme: "Hello World"
Symbol found! Token: RPAREN -- Lexeme: )
Symbol found! Token: SEMICOLON -- Lexeme: ;
Symbol found! Token: WHITESPACE -- Lexeme: 

Symbol found! Token: WHITESPACE -- Lexeme: 

Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: RETURN -- Lexeme: return
Symbol found! Token: WHITESPACE -- Lexeme:  
Symbol found! Token: INTCONST -- Lexeme: 0
Symbol found! Token: SEMICOLON -- Lexeme: ;
Symbol found! Token: WHITESPACE -- Lexeme: 

Symbol found! Token: RBRACE -- Lexeme: }
Symbol found! Token: WHITESPACE -- Lexeme: 

Lexical Analyser enountered an error on line 7 of file helloworld.c -- Token not recognised!
```

## Build

To build --

```bash
git clone https://github.com/timpeskett/c-lex
cd c-lex
make
```

To run -- 
```bash
./lextest <inputfile>
```

## Known Bugs

* The lexical analyser reports an error when it runs into the end of file
  character.
