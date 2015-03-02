#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "linklist.h"
#include "Symbol.h"

typedef Llist SymTable;

void symtableInit( void );

Symbol symtableInsert( Token token, char *lexeme );
Symbol symtableLookup( char *lexeme );

void symtableDestroy( void );

#endif
