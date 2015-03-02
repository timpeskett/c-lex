#include "lexer.h"
#include <stdio.h>
#include "Symbol.h"

char *tokNames[] = {
			"NONE",
			"WHITESPACE",
			"ID",
			"INTCONST",
			"FLOATCONST",
			"ENUMCONST",
			"CHARCONST",
			"STRINGLIT",
			"LBRACE",
			"RBRACE",
			"LSQUARE",
			"RSQUARE",
			"LPAREN",
			"RPAREN",
			"MULTOP",
			"ADDOP",
			"SHIFTOP",
			"RELOP",
			"EQOP",
			"BITANDOP",
			"BITEXOROP",
			"BITINOROP",
			"LOGANDOP",
			"LOGOROP",
			"ASSIGN",
			"INCDEC",
			"DOT",
			"ARROW",
			"COMMA",
			"QUESTION",
			"COLON",
			"ELLIPSIS",
			"SEMICOLON",
			"AUTO",
			"DOUBLE",
			"INT",
			"STRUCT",
			"BREAK",
			"ELSE",
			"LONG",
			"SWITCH",
			"CASE",
			"ENUM",
			"REGISTER",
			"TYPEDEF",
			"CHAR",
			"EXTERN",
			"RETURN",
			"UNION",
			"CONST",
			"FLOAT",
			"SHORT",
			"UNSIGNED",
			"CONTINUE",
			"FOR",
			"SIGNED",
			"VOID",
			"DEFAULT",
			"GOTO",
			"SIZEOF",
			"VOLATILE",
			"DO",
			"IF",
			"STATIC",
			"WHILE"
		};

void init( void )
{
	regexInit();
	symtableInit();
	lexerInit();
}


void destroy()
{
	symtableDestroy();
	lexerDestroy();
	regexDestroy();
}


int main( int argc, char *argv[] )
{
	if( argc < 2 )
	{
		printf(" Usage: lextest <file>\n Lexically analyse a file\n" );
		return 1;
	}

	init();

	if( lexerOpenFile( argv[1] ) )
	{
		Symbol sym;
		sym = lexerNextTok();
		while( sym != NULL )
		{
			printf( "Symbol found! Token: %s -- Lexeme: %s\n", tokNames[sym->token], sym->lexeme );
			sym = lexerNextTok();
		}
		lexerCloseFile();
	}
	else
	{
		printf( "Couldn't open file: %s\n", argv[1] );
	}

	destroy();


	return 0;
}
