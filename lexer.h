#ifndef LEXER_H
#define LEXER_H

#include "global.h"
#include "nfa.h"
#include "symtable.h"
#include "Symbol.h"
#include "regex.h"

#define BUF_SIZE 1024

void lexerInit( void );

Bool lexerOpenFile( char *fName );

Symbol lexerNextTok( void );

void lexerCloseFile( void );

void lexerDestroy( void );



#endif
