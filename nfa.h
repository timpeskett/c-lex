#ifndef NFA_H
#define NFA_H

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "linklist.h"

#define EPSILON (-2)

/* This symbol represents all characters. Any symbol between 256 and 512
 * will be all characters except for the character represented by (char-256).
 * This is for reasons of efficiency */
#define ALL (256)
#define NUMSYMBOLS (256) /*0-255*/

#define DEFCAPACITY 10

#define MARKED 1
#define UNMARKED 0

typedef struct
{
	Bool start;
	Bool finish;
	Llist transitions;
} FAstate;	


typedef struct
{
	int symbol;
	int destNum;
} FAtransition;

typedef struct
{
	int *states;
	int numStates;
} FAstateinfo;

typedef struct
{
	FAstate *states;
	int numStates;
	int stateCapacity;
} *Nfa;

/* A Dfa is really just a special type of Nfa. Shhhh don't tell anyone */
typedef Nfa Dfa;


Nfa nfaCreateEmpty( void );
int nfaAddEmptyState( Nfa nfa, Bool isStart, Bool isFinish );
/* From is from and to are states */
void nfaAddTransition( Nfa nfa, int from, int to, int symbol, Bool isAll );
/* Returns a null-terminated list of all characters in the alphabet. Must be freed after use */
int *nfaGetAlphabet( Nfa nfa );
void nfaSetStart( Nfa nfa, int state, Bool isStart );
void nfaSetFinish( Nfa nfa, int state, Bool isFinish );
/* Must call nfaDestroyData to destroy the FAstateinfo once it is finished with */
FAstateinfo *nfaGetFinishStates( Nfa nfa );
FAstateinfo *nfaGetStartStates( Nfa nfa );
void nfaDestroyData( FAstateinfo *inData );
void nfaDestroyNfa( Nfa nfa );

/* Functions will return null if operation is not possible */
Nfa nfaConcat( Nfa n1, Nfa n2 );
Nfa nfaKleene( Nfa n1 );
Nfa nfaAlternate( Nfa n1, Nfa n2 );

/* These functions have the same effect but they do the operation in place instead of creating a new nfa */
void nfaConcatInPlace( Nfa dest, Nfa src );
void nfaKleeneInPlace( Nfa n1 );
void nfaAlternateInPlace( Nfa dest, Nfa src );

/* Creates and NFA with two states. start state and finish state which transitions on symbol */
Nfa nfaCreateBasic( int symbol );

/* Functions for working with DFAs */
Dfa nfaToDfa( Nfa nfa );
Bool nfaSimulateDfa( Dfa dfa, char *string );
/* Returns the last number of characters that put the dfa into
 * and accepting state. only terminates when there are no transitions out
 * of the current state */
int nfaAcceptLongestDfa( Dfa dfa, int (*nextChar)( void ) );
void nfaDestroyDfa( Dfa dfa );

void DBGPRINTNFA( Nfa nfa );
#endif
