#include "symtable.h"

static SymTable symtable = NULL;
static void createSymTable();
static void destroySymTable();




void symtableInit( void )
{
	if( symtable == NULL )
	{
		createSymTable();
	}
}

void symtableDestroy( void )
{
	destroySymTable();
}

void createSymTable()
{
	symtable = createLinkList();

	symtableInsert( AUTO ,"auto" );
	symtableInsert( DOUBLE ,"double" );
	symtableInsert( INT ,"int" );
	symtableInsert( STRUCT ,"struct" );
	symtableInsert( BREAK ,"break" );
	symtableInsert( ELSE ,"else" );
	symtableInsert( LONG ,"long" );
	symtableInsert( SWITCH ,"switch" );
	symtableInsert( CASE ,"case" );
	symtableInsert( ENUM ,"enum" );
	symtableInsert( REGISTER ,"register" );
	symtableInsert( TYPEDEF ,"typedef" );
	symtableInsert( CHAR ,"char" );
	symtableInsert( EXTERN ,"extern" );
	symtableInsert( RETURN ,"return" );
	symtableInsert( UNION ,"union" );
	symtableInsert( CONST ,"const" );
	symtableInsert( FLOAT ,"float" );
	symtableInsert( SHORT ,"short" );
	symtableInsert( UNSIGNED ,"unsigned" );
	symtableInsert( CONTINUE ,"continue" );
	symtableInsert( FOR ,"for" );
	symtableInsert( SIGNED ,"signed" );
	symtableInsert( VOID ,"void" );
	symtableInsert( DEFAULT ,"default" );
	symtableInsert( GOTO ,"goto" );
	symtableInsert( SIZEOF ,"sizeof" );
	symtableInsert( VOLATILE ,"volatile" );
	symtableInsert( DO ,"do" );
	symtableInsert( IF ,"if" );
	symtableInsert( STATIC ,"static" );
	symtableInsert( WHILE ,"while" );
}

void destroySymTable()
{
	Symbol sym;

	while( ( sym = removeLinkListItem( symtable, 0 ) ) != NULL )
	{
		if( sym )
		{
			destroySymbol( sym );
		}
	}
	destroyLinkList( symtable );
}

Symbol symtableInsert( Token token, char *lexeme )
{
	Symbol newSym;

	assert( symtable != NULL );

	newSym = createSymbol( token, lexeme );

	insertEndLinkListItem( symtable, newSym );

	return newSym;
}

Symbol symtableLookup( char *lexeme )
{
	Symbol found, current;
	int position;

	assert( symtable != NULL );

	found = NULL;
	position = 0;

	while( ( current = getLinkListItem( symtable, position ) ) != NULL )
	{
		if( strcmp( lexeme, current->lexeme ) == 0 )
		{
			found = current;
			break;
		}
		position++;
	}

	return found;
}
