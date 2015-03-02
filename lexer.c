#include "lexer.h"

static Bool initiated = false;
static struct
{
	char *regexp;
	Token tok;
	Dfa dfa;
} dfaInfo[] = 	{
			{ "(\\ |\\t|\\n)", WHITESPACE, NULL },
			{ "({)", LBRACE, NULL },
			{ "(})", RBRACE, NULL },
			{ "([)", LSQUARE, NULL },
			{ "(])", RSQUARE, NULL },
			{ "(\\()", LPAREN, NULL },
			{ "(\\))", RPAREN, NULL },
			{ "(\\*|/|%)", MULTOP, NULL },
			{ "(+|-)", ADDOP, NULL },
			{ "(<<|>>)", SHIFTOP, NULL },
			{ "(<|>|<=|>=)", RELOP, NULL },
			{ "(==|!=)", EQOP, NULL },
			{ "(&)", BITANDOP, NULL },
			{ "(^)", BITEXOROP, NULL },
			{ "(\\|)", BITINOROP, NULL },
			{ "(&&)", LOGANDOP, NULL },
			{ "(\\|\\|)", LOGOROP, NULL },
			{ "(=|\\*=|/=|%=|+=|-=|<<=|>>=|&=|^=|\\|=)", ASSIGN, NULL },
			{ "(++|--)", INCDEC, NULL },
			{ "(.)", DOT, NULL },
			{ "(->)", ARROW, NULL },
			{ "(,)", COMMA, NULL },
			{ "(?)", QUESTION, NULL },
			{ "(:)", COLON, NULL },
			{ "(...)", ELLIPSIS, NULL },
			{ "(;)", SEMICOLON },
			{ "((_|\\l)(\\l|_|\\d)*)", ID, NULL },
			{ "(((1|2|3|4|5|6|7|8|9)(\\d)*)|(0(\\o)*)|(0(x|X)(\\x)(\\x)*))", INTCONST, NULL },
			/* Sorry */
			{ "((\\d\\d*(e|E)(-|+)\\d\\d*)|(\\d\\d*(e|E)\\d\\d*)|(\\d\\d*(e|E)(-|+)\\d\\d*(f|l|F|L))|(\\d\\d*(e|E)\\d\\d*(f|l|F|L))|(\\d\\d*.)|(\\d\\d*.(e|E)\\d\\d*)|(\\d\\d*.(e|E)(+|-)\\d\\d*)|(\\d\\d*.(f|l|F|L))|(\\d\\d*.(e|E)\\d\\d*(f|l|F|L))|(\\d\\d*.(e|E)(+|-)\\d\\d*(f|l|F|L))|(\\d*.\\d\\d*)|(\\d*.\\d\\d*(e|E)\\d\\d*)|(\\d*.\\d\\d*(e|E)(+|-)\\d\\d*)|(\\d*.\\d\\d*(f|l|F|L))|(\\d*.\\d\\d*(e|E)\\d\\d*(f|l|F|L))|(\\d*.\\d\\d*(e|E)(+|-)\\d\\d*(f|l|F|L)))", FLOATCONST, NULL },
			{ "(\"(\\e\"|\\\\\")*\")", STRINGLIT, NULL },
			{ "(('(\\e'|\\\\')*')|(L('(\\e'|\\\\')*')))", CHARCONST, NULL }

		};

/* Structure to hold the data about our currently open file */
static struct 
{ 
	FILE *inFile; 
	char *fName; 
	int lineno; 
	int fSize; 
} fData;
/* curPos is index into buffer. startPos is the position in the file of the buffer */
static struct 
{ 
	unsigned char buffer[BUF_SIZE]; 
	int curPos; 
	int startPos; 
} buf; 

/* These need to be constructed once and then killed once. Their lifetime is the entire program */
static void loadDFAs( void );
static void destroyDFAs( void );

/* failstart is the number of the start state of the DFA that we failed on */
static void errormsg( char *msg );

/* Returns EOF at end of file */
static int nextchar( void );

/* true on success. false of fail */
static Bool movepointer( int dx );


void lexerInit( void )
{
	if( initiated != true )
	{
		loadDFAs();
		initiated = true;
	}
}

void lexerDestroy( void )
{
	destroyDFAs();
	initiated = false;
}


void loadDFAs( void )
{
	int numDFAs;
	int i;

	numDFAs = sizeof( dfaInfo ) / sizeof( *dfaInfo );
        for( i = 0; i < numDFAs; i++ )
	{
		dfaInfo[i].dfa = regexToDFA( dfaInfo[i].regexp );
	}
}

void destroyDFAs( void )
{
	int numDFAs;
	int i;

	numDFAs = sizeof( dfaInfo ) / sizeof( *dfaInfo );
	for( i = 0; i < numDFAs; i++ )
	{
		nfaDestroyDfa( dfaInfo[i].dfa );
		dfaInfo[i].dfa = NULL;
	}
}


Bool lexerOpenFile( char *fName )
{
	Bool result = false;
	
	assert( fData.inFile == NULL );
	assert( initiated );
	
	/* The b is needed for windows compatibility */
	fData.inFile = fopen( fName, "rb" );
	if( fData.inFile != NULL )
	{
		fData.fName = malloc( sizeof( *fData.fName ) * ( strlen( fName ) + 1 ) );
		strcpy( fData.fName, fName );
		fData.lineno = 1;
		
		fseek( fData.inFile, 0, SEEK_END );
		fData.fSize = ftell( fData.inFile );
		fseek( fData.inFile, 0, SEEK_SET );

		buf.curPos = 0;
		buf.startPos = 0;
		fread( buf.buffer, sizeof( *buf.buffer ), BUF_SIZE, fData.inFile );	

		result = true;
	}


	return result;
}

Symbol lexerNextTok( void )
{
	int maxEnt, maxChars;
	Symbol outSym = NULL;
	int bufStartPos;
	int i;
	int numDFAs;

	bufStartPos = buf.startPos + buf.curPos;

	maxEnt = -1;
	maxChars = 0;

	numDFAs = sizeof( dfaInfo ) / sizeof( *dfaInfo );
	for( i = 0; i < numDFAs; i++ )
	{
		int numAccepted;

		/* nextchar is function pointer*/
		numAccepted = nfaAcceptLongestDfa( dfaInfo[i].dfa, &nextchar );
		if( numAccepted > maxChars )
		{
			maxChars = numAccepted;
			maxEnt = i;
		}
		/* Reset the character pointer in the buffer */
		movepointer( bufStartPos - (buf.startPos + buf.curPos) );
	}

	if( maxEnt == -1 )
	{
		errormsg( "Token not recognised!" );
	}
	else
	{
		char *lexeme;

		lexeme = malloc( ( maxChars + 1 ) * sizeof( *lexeme ) );
		for( i = 0; i < maxChars; i++ )
		{
			lexeme[i] = nextchar();
		}
		lexeme[i] = '\0';


		if( dfaInfo[maxEnt].tok == WHITESPACE )
		{
			if( lexeme[0] == '\n' )
			{
				fData.lineno++;
			}
		}
		
		/* Check to see if symbol is already in our symbol table, if not then
		 * put it in. All of our language keywords (if/while/do/etc) will all be
		 * in the symbol table from the beginning */
		if( ( outSym = symtableLookup( lexeme ) ) == NULL )
		{
			outSym = symtableInsert( dfaInfo[maxEnt].tok, lexeme );
		}
		free( lexeme );
	}

	return outSym;
}


void lexerCloseFile( void )
{
	if( fData.inFile != NULL )
	{
		fclose( fData.inFile );
		fData.inFile = NULL;
		free( fData.fName );
		fData.fName = NULL;
	}
}


void errormsg( char *msg )
{
	printf( "Lexical Analyser enountered an error on line %d of file %s -- %s\n", fData.lineno, fData.fName, msg );
}

/* Returns EOF at end of file */
int nextchar( void )
{
	int outChar;

	assert( 0 <= buf.curPos && buf.curPos < BUF_SIZE );

	if( buf.startPos + buf.curPos >= fData.fSize )
	{
		/* We still want to increment the pointer here so that 
		 * it can be reset properly */
		outChar = EOF;
	}
	else
	{
		outChar = buf.buffer[buf.curPos];
	}

	movepointer(1);

	return outChar;
}


/* true on success. false of fail */
Bool movepointer( int dx )
{
	int startPos, curPos;
	Bool result = true;

	/* Check to see if we need to move our buffer */
	if( buf.curPos + dx < 0 || buf.curPos + dx >= BUF_SIZE )
	{
		/* Our new startpos will leave the pointer in the middle of
		 * the buffer pointing at the same place in the file as before */
		startPos = buf.startPos + buf.curPos + dx - BUF_SIZE / 2;
		/* New curPos is halfway through the buffer to reduce file reads */	
		curPos = BUF_SIZE / 2;	
		if( startPos < 0 )
		{
			curPos += ( 0 - startPos ) % BUF_SIZE;
			startPos = 0;
		}

		/* The file pointer is actually at the end of our buffer so
		 * we need to subtract BUF_SIZE */
		if( fseek( fData.inFile, startPos - ( buf.startPos + BUF_SIZE ), SEEK_CUR ) == 0 )
		{
			buf.startPos = startPos;
			buf.curPos = curPos;
			fread( buf.buffer, sizeof( *buf.buffer ), BUF_SIZE, fData.inFile );
		}
		else
		{
			result = false;
		}
	}
	else
	{
		buf.curPos += dx;
	}

	return result;
}
