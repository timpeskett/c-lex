#include "regex.h"

static int token;
static const char *regexp;

static const char symbols[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&-=_+`~/,/<>?;':\"[]{}.";
/* The character that we use to escape special charaters. In this case it is a single backslash */
static const char escape = '\\';
/* our special characters. Those that are used to specifiy regexps and:
 * 	t = tab
 * 	n = newline
 * 	d = decimal digit (0-9)
 * 	x = hex digit (0-9A-Fa-f)
 * 	o = octal digit (0-7)
 * 	l = letter (a-zA-Z)
 * 	c = any character (0-255)
 * 	e = any character EXCEPT for the character immediately following. e.g \e"a will match
 * any character except a quote and then will match 'a'
 * 	*/
static const char special[] = "*()|ctn\\dxol e";

/* These are nfas that are built at the beginning and then used to supply the NFAs for special characters */
Bool initiated = false;
static struct
{
	char special;
	char *regex;
	Nfa nfa;
} 	prebuilts[] =	{	
				{ 'd', "(0|1|2|3|4|5|6|7|8|9)", NULL },
				{ 'x', "(0|1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|a|b|c|d|e|f)", NULL },
				{ 'o', "(0|1|2|3|4|5|6|7)", NULL },
				{ 'l', "(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)", NULL },
			};


static void match( int lookup );
static void start( Nfa nfa );
static void unit( Nfa nfa );
static void unit_seq( Nfa nfa );
static void alternate( Nfa nfa );
static void kleene( Nfa nfa );
static void symbol_seq( Nfa nfa );
static void symbol( Nfa nfa );
static Bool isSymbol( char test ); 

void regexInit( void )
{
	int i;
	int numNfas;

	if( initiated == false )
	{
		/* Initialise the nfas for our special characters if not already done */
		numNfas = sizeof( prebuilts ) / sizeof( *prebuilts );
		for( i = 0; i < numNfas; i++ )
		{
			prebuilts[i].nfa = nfaCreateEmpty();
			regexp = prebuilts[i].regex;
			token = *regexp;
			start( prebuilts[i].nfa );
		}

		initiated = true;
	}
}

void regexDestroy( void )
{
	int i, numNfas;

	numNfas = sizeof( prebuilts ) / sizeof( *prebuilts );
	for( i = 0; i < numNfas; i++ )
	{
		nfaDestroyNfa( prebuilts[i].nfa );
	}
	initiated = false;
}

Dfa regexToDFA( char *inRegEx )
{
	Nfa nfa;
	Dfa dfa;

	assert( initiated );
		
	nfa = nfaCreateEmpty();

	regexp = inRegEx;
	token = *regexp;

	start( nfa );

	dfa = nfaToDfa( nfa );

	nfaDestroyNfa( nfa );

	return dfa;
}


void match( int lookup )
{
	if( lookup == token )
	{
		regexp++;
		token = *regexp;
	}
	else
	{
		printf( "Regex: Error occurred during parsing on token %d (%c)\n", token, token );
	}
}

void start( Nfa nfa )
{
	if( token == '(' || isSymbol( token ) )
	{
		unit( nfa ); unit_seq( nfa );
	}
	else
	{
		printf( "Regex: Error occurred\n" );
	}
}

void unit( Nfa nfa )
{
	Nfa temp;

	temp = nfaCreateEmpty();
	if( token == '(' )
	{
		match( '(' ); unit( temp ); unit_seq( temp ); match( ')' ); kleene( temp ); alternate( temp ); 
		nfaConcatInPlace( nfa, temp );
	}
	else if( isSymbol( token ) )
	{
		symbol( temp ); kleene( temp ); symbol_seq( temp ); alternate( temp );
		nfaConcatInPlace( nfa, temp );
	}
	else
	{
		printf( "Regex: Error occured while parsing unit\n" );
	}
	nfaDestroyNfa( temp );
}	

static void unit_seq( Nfa nfa )
{
	Nfa temp;

	if( token == '(' || isSymbol( token ) )
	{
		temp = nfaCreateEmpty();
		unit( temp ); unit_seq( temp );
		nfaConcatInPlace( nfa, temp );
	}
}


void alternate( Nfa nfa )
{
	Nfa temp;

	if( token == '|' )
	{
		temp = nfaCreateEmpty();
		match( '|' ); unit( temp ); alternate( temp );
		nfaAlternateInPlace( nfa, temp );
		nfaDestroyNfa( temp );
	}
}


void kleene( Nfa nfa )
{
	if( token == '*' )
	{
		nfaKleeneInPlace( nfa );
		match( '*' );
	}
}


void symbol_seq( Nfa nfa )
{
	Nfa temp;

	if( isSymbol( token ) )
	{
		temp = nfaCreateEmpty();
		symbol( temp ); kleene( temp );symbol_seq( temp );
		nfaConcatInPlace( nfa, temp );
		nfaDestroyNfa( temp );
	}
}


Bool isSymbol( char test )
{
	Bool result = false;

	/* strchr accepts 0 but we don't want that */
	if( test != 0 && ( strchr( symbols, test ) != NULL || test == escape ) )
	{
		result = true;
	}

	return result;
}


void symbol( Nfa nfa )
{
	Nfa basic;

	if( strchr( symbols, token ) != NULL )
	{
		basic = nfaCreateBasic( token );
		nfaConcatInPlace( nfa, basic );
		nfaDestroyNfa( basic );

		match( token );
	}
	else if( token == escape )
	{
		match( escape );
		if( token != 0 && strchr( special, token ) != NULL )
		{
			int i, numNfas;
			Bool matched;

			matched = false;
			i = 0;
			numNfas = sizeof( prebuilts ) / sizeof( *prebuilts );
			while( matched == false && i < numNfas )
			{	
				if( prebuilts[i].special == token )
				{
					matched = true;
				}
				i++;
			}
			
			/* If matched is true then special is one of our prebuilt nfas */
			if( matched == true )
			{
				i--;
				nfaConcatInPlace( nfa, prebuilts[i].nfa );
			}
			else
			{
				switch( token )
				{
					case 't': /* Tab character */
						basic = nfaCreateBasic( '\t' );
						break;
					case 'n': /* Newline character */
						basic = nfaCreateBasic( '\n' );
						break;
					case 'c':
						/*basic = nfaCreateBasic( 1 );
						for( i = 2; i <= 0xFF; i++ )
						{
							Nfa nextChar;
							nextChar = nfaCreateBasic( i );
							nfaAlternateInPlace( basic, nextChar );
							nfaDestroyNfa( nextChar );
						}*/
						basic = nfaCreateBasic( ALL );
						break;
					case 'e':
						match( 'e' );
						basic = nfaCreateBasic( ALL + token );
						/*basic = nfaCreateBasic( 0 );
						for( i = 1; i < 0xFF; i++ )
						{
							Nfa nextChar;
							if( i != token )
							{
								nextChar = nfaCreateBasic( i );
								nfaAlternateInPlace( basic, nextChar );
								nfaDestroyNfa( nextChar );
							}
						}*/
						break;	
					default:
						basic = nfaCreateBasic( token );
						break;
				}
				nfaConcatInPlace( nfa, basic );
				nfaDestroyNfa( basic );
				
			}
			match( token );
		}
		else
		{
			printf( "Regex.c: Error occurred you escaped wrong\n" );
		}
			
	}
	else
	{
		printf( "Regex.c: Error occured, expected letter but got %d (%c)\n", token, token );
	}
}
