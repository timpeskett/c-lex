#include "Symbol.h"


Symbol createSymbol( Token tok, char *lex )
{
	Symbol outSym;

	outSym = malloc( sizeof( *outSym ) );
	outSym->token = tok;
	outSym->lexeme = malloc( sizeof( *outSym->lexeme ) * ( strlen( lex ) + 1 ) );
	strcpy( outSym->lexeme, lex );

	return outSym;
}


void destroySymbol( Symbol sym )
{
	assert( sym != NULL );
	free( sym->lexeme );
	free( sym );
}
