#include "nfa.h"

static Bool isValidSymbol( int symbol );


Nfa nfaCreateEmpty( void )
{
	Nfa nfa;

	nfa = malloc( sizeof( *nfa ) );
       	nfa->numStates = 0;
	nfa->stateCapacity = DEFCAPACITY;
	nfa->states = malloc( sizeof( *nfa->states ) * nfa->stateCapacity );	

	return nfa;
}


int nfaAddEmptyState( Nfa nfa, Bool isStart, Bool isFinish )
{
	assert( nfa != NULL );

	if( nfa->numStates >= nfa->stateCapacity )
	{
		FAstate *newStates;
		int i;

		newStates = malloc( sizeof( *newStates ) * ( nfa->stateCapacity + nfa->stateCapacity ) );
		for( i = 0; i < nfa->numStates; i++ )
		{
			newStates[i] = nfa->states[i];
		}
		free( nfa->states );
		nfa->states = newStates;
		nfa->stateCapacity = nfa->stateCapacity + nfa->stateCapacity;
	}

	nfa->numStates++;

	nfaSetStart( nfa, nfa->numStates - 1, isStart );
	nfaSetFinish( nfa, nfa->numStates - 1, isFinish );
	nfa->states[nfa->numStates - 1].transitions = createLinkList();

	return nfa->numStates - 1;
}

void nfaSetStart( Nfa nfa, int state, Bool isStart )
{
	assert( 0 <= state && state  < nfa->numStates );
	
	nfa->states[state].start = isStart;
}

void nfaSetFinish( Nfa nfa, int state, Bool isFinish )
{
	assert( 0 <= state && state  < nfa->numStates );

	nfa->states[state].finish = isFinish;
}


/* From is from and to are states. Is all determines whether
 * the supplied symbol is the ALL symbol or is between ALL and
 * ALL + NUMSYMBOLS */
void nfaAddTransition( Nfa nfa, int from, int to, int symbol, Bool isAll )
{
	FAtransition *newTrans;

	assert( nfa != NULL );
	assert( 0 <= from && from < nfa->numStates );
	assert( 0 <= to && to < nfa->numStates );

	newTrans = malloc( sizeof( *newTrans ) );
	newTrans->symbol = symbol;
	newTrans->destNum = to;

	if( isAll )
	{
		insertEndLinkListItem( nfa->states[from].transitions, newTrans );
	}
	else
	{
		insertStartLinkListItem( nfa->states[from].transitions, newTrans );
	}
}


int *nfaGetAlphabet( Nfa nfa )
{
	int i, j, k;
	int numTrans;
	int *outAlphabet;
	Bool alrdyPresent;

	/* Compute an upper bound for the number of symbols in the alphabet */
	numTrans = 0;
	for( i = 0; i < nfa->numStates; i++ )
	{
		numTrans += getLinkListCount( nfa->states[i].transitions );
	}
	outAlphabet = malloc( ( numTrans + 1 ) *  sizeof( *outAlphabet ) );
	memset( outAlphabet, '\0', sizeof( *outAlphabet ) * ( numTrans + 1 ) );

	for( i = 0; i < nfa->numStates; i++ )
	{
		numTrans = getLinkListCount( nfa->states[i].transitions );
		for( j = 0; j < numTrans; j++ )
		{
			int sym;
			
			/* Get next transition symbol and add to list if not already present */
			alrdyPresent = false;
			sym = ( (FAtransition*)getLinkListItem( nfa->states[i].transitions, j ) )->symbol;
			if( sym != EPSILON )
			{
				k = 0;
				while( outAlphabet[k] != '\0' && alrdyPresent == false )
				{
					if( outAlphabet[k] == sym )
					{
						alrdyPresent = true;
					}
					k++;
				}
				if( !alrdyPresent )
				{
					outAlphabet[k] = sym;
				}
			}
		}
	}

	return outAlphabet;
}


/* Must call nfaDestroyData to destroy the array once it is finished with */
FAstateinfo *nfaGetFinishStates( Nfa nfa )
{
	FAstateinfo *newInfo;
	int i, stored;

	assert( nfa != NULL );

	newInfo = malloc( sizeof( *newInfo ) );

	newInfo->numStates = 0;
	for( i = 0; i < nfa->numStates; i++ )
	{
		if( nfa->states[i].finish )
		{
			newInfo->numStates++;
		}
	}

	newInfo->states = malloc( sizeof( *newInfo->states ) * newInfo->numStates );
	stored = 0;
	for( i = 0; i < nfa->numStates; i++ )
	{
		if( nfa->states[i].finish )
		{
			newInfo->states[stored] = i;
			stored++;
		}
	}

	return newInfo;
}


FAstateinfo *nfaGetStartStates( Nfa nfa )
{
	FAstateinfo *newInfo;
	int i, stored;

	assert( nfa != NULL );

	newInfo = malloc( sizeof( *newInfo ) );

	newInfo->numStates = 0;
	for( i = 0; i < nfa->numStates; i++ )
	{
		if( nfa->states[i].start )
		{
			newInfo->numStates++;
		}
	}

	newInfo->states = malloc( sizeof( *newInfo->states ) * newInfo->numStates );
	stored = 0;
	for( i = 0; i < nfa->numStates; i++ )
	{
		if( nfa->states[i].start )
		{
			newInfo->states[stored] = i;
			stored++;
		}
	}

	return newInfo;
}

void nfaDestroyData( FAstateinfo *inData )
{
	if( inData != NULL )
	{
		free( inData->states );
	}
	free( inData );
}

/* Internal function used to copy the states of one NFA to another. Makes copies of ALL data. Does not create any new transitions it just copies the states into the NFAs. Makes to distinct NFAs into one NFA with two disjoint sets of states */
static void nfaConcatStates( Nfa dest, Nfa src )
{
	int startSize;
	int i, j;
	Bool isAll;

	startSize = dest->numStates;

	/* These need to be added first so that our transitions work properly */
	for( i = 0; i < src->numStates; i++ )
	{
		nfaAddEmptyState( dest, src->states[i].start, src->states[i].finish );
	}

	for( i = 0; i < src->numStates; i++ )
	{
		int numTrans;
		numTrans = getLinkListCount( src->states[i].transitions );
		for( j = 0; j < numTrans; j++ )
		{
			FAtransition *fatrans;

			fatrans = getLinkListItem( src->states[i].transitions, j );
			isAll = ( ALL <= fatrans->symbol && fatrans->symbol < ALL + NUMSYMBOLS );
			nfaAddTransition( dest, i + startSize, fatrans->destNum + startSize, fatrans->symbol, isAll );
		}
	}
}

/* Functions will return null if operation is not possible */
Nfa nfaConcat( Nfa n1, Nfa n2 )
{
	Nfa new;
	int i, j;
	FAstateinfo *fin1, *start2;

	if( n1 != NULL && n2 != NULL )
	{
		/* Create the new NFA and add the old states to it*/
		new = nfaCreateEmpty();
		nfaConcatStates( new, n1 );
		nfaConcatStates( new, n2 );

		/* Make epsilon transitions from the accept states of n1 to the start states of n2 */
		fin1 = nfaGetFinishStates( n1 );
		start2 = nfaGetStartStates( n2 );
		for( i = 0; i < fin1->numStates; i++ )
		{
			/* finish states of n1 are no longer finish states */
			nfaSetFinish( new, fin1->states[i], false );
			for( j = 0; j < start2->numStates; j++ )
			{
				nfaSetStart( new, start2->states[j], false );
				nfaAddTransition( new, fin1->states[i], n1->numStates + start2->states[j], EPSILON, false );
			}
		}	
		nfaDestroyData( fin1 );
		nfaDestroyData( start2 );
	}
	else
	{
		new = NULL;
	}

	return new;
}
	
Nfa nfaKleene( Nfa n1 )
{
	Nfa new;
	int start, finish;
	FAstateinfo *startStates, *finStates;	
	int i, j;

	if( n1 != NULL )
	{
		new = nfaCreateEmpty();
		start = nfaAddEmptyState( new, true, false );
		finish = nfaAddEmptyState( new, false, true );

		nfaConcatStates( new, n1 );

		startStates = nfaGetStartStates( new );
		finStates = nfaGetFinishStates( new );

		nfaAddTransition( new, start, finish, EPSILON, false );

		for( i = 0; i < finStates->numStates; i++ )
		{
			if( finStates->states[i] != finish )
			{
				nfaSetFinish( new, finStates->states[i], false );
				nfaAddTransition( new, finStates->states[i], finish, EPSILON, false );
				for( j = 0; j < startStates->numStates; j++ )
				{
					if( startStates->states[j] != start )
					{
						nfaAddTransition( new, finStates->states[i], startStates->states[j], EPSILON, false );
					}
				}
			}
		}

		for( i = 0; i < startStates->numStates; i++ )
		{
			if( startStates->states[i] != start )
			{
				nfaSetStart( new, startStates->states[i], false );
				nfaAddTransition( new, start, startStates->states[i], EPSILON, false );
			}
		}
		nfaDestroyData( startStates );
		nfaDestroyData( finStates );
	}
	else
	{
		new = NULL;
	}

	return new;
}
	

Nfa nfaAlternate( Nfa n1, Nfa n2 )
{
	Nfa new;
	int newStart, newFin;
	FAstateinfo *starts, *fins;
	int i;

	if( n1 != NULL && n2 != NULL )
	{
		new = nfaCreateEmpty();	

		newStart = nfaAddEmptyState( new, true, false );
		newFin = nfaAddEmptyState( new, false, true );
		nfaConcatStates( new, n1 );
		nfaConcatStates( new, n2 );

		starts = nfaGetStartStates( new );
		fins = nfaGetFinishStates( new );
		
		for( i = 0; i < starts->numStates; i++ )
		{
			if( starts->states[i] != newStart )
			{
				nfaAddTransition( new, newStart, starts->states[i], EPSILON, false );
				nfaSetStart( new, starts->states[i], false );
			}
		}

		for( i = 0; i < fins->numStates; i++ )
		{
			if( fins->states[i] != newFin )
			{
				nfaAddTransition( new, fins->states[i], newFin, EPSILON, false );
				nfaSetFinish( new, fins->states[i], false );
			}
		}
		nfaDestroyData( starts );
		nfaDestroyData( fins );
	}
	else
	{
		new = NULL;
	}

	return new;
}



/* These functions have the same effect but they do the operation in place instead of creating a new nfa */
void nfaConcatInPlace( Nfa dest, Nfa src )
{
	int i, j;
	FAstateinfo *fin1, *start2;

	if( dest != NULL && src != NULL )
	{
		fin1 = nfaGetFinishStates( dest );
		start2 = nfaGetStartStates( src );
		/* Make all the start state positions relative to the new dest rather than the src */
		for( i = 0; i < start2->numStates; i++ )
		{
			start2->states[i] += dest->numStates;
		}	
		nfaConcatStates( dest, src ); 

		/* Make epsilon transitions from the accept states of dest to the start states of src */
		for( i = 0; i < fin1->numStates; i++ )
		{
			/* finish states of dest are no longer finish states */
			nfaSetFinish( dest, fin1->states[i], false );
			for( j = 0; j < start2->numStates; j++ )
			{
				/*start states of src are no longer start states */
				nfaSetStart( dest, start2->states[j], false );
				nfaAddTransition( dest, fin1->states[i], start2->states[j], EPSILON, false );
			}
		}	
		nfaDestroyData( fin1 );
		nfaDestroyData( start2 );
	}
}


void nfaKleeneInPlace( Nfa n1 )
{
	int start, finish;
	FAstateinfo *startStates, *finStates;	
	int i, j;

	if( n1 != NULL )
	{
		startStates = nfaGetStartStates( n1 );
		finStates = nfaGetFinishStates( n1 );

		start = nfaAddEmptyState( n1, true, false );
		finish = nfaAddEmptyState( n1, false, true );


		nfaAddTransition( n1, start, finish, EPSILON, false );

		for( i = 0; i < finStates->numStates; i++ )
		{
			nfaSetFinish( n1, finStates->states[i], false );
			nfaAddTransition( n1, finStates->states[i], finish, EPSILON, false );
			for( j = 0; j < startStates->numStates; j++ )
			{
					nfaAddTransition( n1, finStates->states[i], startStates->states[j], EPSILON, false );
			}
		}

		for( i = 0; i < startStates->numStates; i++ )
		{
			nfaSetStart( n1, startStates->states[i], false );
			nfaAddTransition( n1, start, startStates->states[i], EPSILON, false );
		}
		nfaDestroyData( startStates );
		nfaDestroyData( finStates );
	}
}



void nfaAlternateInPlace( Nfa dest, Nfa src )
{
	int newStart, newFin;
	FAstateinfo *starts, *fins;
	int i;

	if( dest != NULL && src != NULL )
	{
		nfaConcatStates( dest, src );
		starts = nfaGetStartStates( dest );
		fins = nfaGetFinishStates( dest );

		newStart = nfaAddEmptyState( dest, true, false );
		newFin = nfaAddEmptyState( dest, false, true );
		
		/* Make new start start point to all start states and then unset old start states */
		for( i = 0; i < starts->numStates; i++ )
		{
			nfaAddTransition( dest, newStart, starts->states[i], EPSILON, false );
			nfaSetStart( dest, starts->states[i], false );
		}

		/* Make new finish state pointed to by all finish states and the nunset old finish */
		for( i = 0; i < fins->numStates; i++ )
		{
			nfaAddTransition( dest, fins->states[i], newFin, EPSILON, false );
			nfaSetFinish( dest, fins->states[i], false );
		}
		nfaDestroyData( starts );
		nfaDestroyData( fins );
	}
}


void nfaDestroyNfa( Nfa nfa )
{
	int i, j;

	if( nfa != NULL )
	{
		if( nfa->states != NULL )
		{
			for( i = 0; i < nfa->numStates; i++ )
			{
				int numTrans;
				numTrans = getLinkListCount( nfa->states[i].transitions );
				for( j = 0; j < numTrans; j++ )
				{
					free( removeLinkListItem( nfa->states[i].transitions, 0 ) );
				}
				destroyLinkList( nfa->states[i].transitions );
			}
		}
		free( nfa->states );
	}
	free( nfa );
}



/* Creates and NFA with two states. start state and finish state which transitions on symbol */
Nfa nfaCreateBasic( int symbol )
{
	Nfa new;
	int start, finish;

	new = nfaCreateEmpty();
	start = nfaAddEmptyState( new, true, false );
	finish = nfaAddEmptyState( new, false, true );

	/* we don't have to sweat about the ALL symbol here because
	 * this is guaranteed to be the only transition so far */
	nfaAddTransition( new, start, finish, symbol, false );

	return new;
}


/************************************************
 * 		The following few function are
 * 		present to facilitate conversion
 * 		of an NFA into a DFA. They are:
 * 		 * addState - Adds a state to a set
 * 		 	of states (no duplicates)
 * 		 *epsClosureRecurse - A recursive 
 * 		 	helper function to compute
 * 		 	the epsilon closure of a state
 * 		 	in an NFA
 * 		 * epsClosure - A wrapper for epsClosureRecurse
 * 		 * epsClosureSet - Computes the epsilon closure
 * 		 	of a set of states
 * 		 * move - Computes the set of states that
 * 		 	a state can transition to on a
 * 		 	given input symbol
 * 		 * findFAstate - searches an array of FAstateinfos
 * 		 	to see if it contains a certain FAstateinfo
 * 		 	order does not matter here
 * 		 	returns the index of the found FAstateinfo
 * 		 	returns -1 if not found
 * 		 * doesContainFinish - searches an FAstateinfo to see
 * 		 	if any of the states it references are finish
 * 		 	states
 */
 

/* Will only add the state if not already present */
static void addState( FAstateinfo *fsi, int stateNum )
{
	int i;
	Bool alrdyPresent = false;

	i = 0;
	while( i < fsi->numStates && !alrdyPresent )
	{
		if( fsi->states[i] == stateNum )
		{
			alrdyPresent = true;
		}
		i++;
	}
	if( !alrdyPresent )
	{
		fsi->states[fsi->numStates] = stateNum;
		fsi->numStates++;
	}
}


/* Recursive helper function for epsClosure */
static void epsClosureRecurse( Nfa nfa, int curStateNum, FAstateinfo *outStates )
{
	int i, j, numTrans;
	Bool alrdyPresent;
	FAstate *curState;

	curState = &nfa->states[curStateNum];

	numTrans = getLinkListCount( curState->transitions );
	for( i = 0; i < numTrans; i++ )
	{
		FAtransition *trans;

		alrdyPresent = false;
		trans = getLinkListItem( curState->transitions, i );
		j = 0;
		while( j < outStates->numStates && alrdyPresent == false )
		{
			if( trans->destNum == outStates->states[j] )
			{
				alrdyPresent = true;
			}
			j++;
		}
		if( !alrdyPresent && trans->symbol == EPSILON )
		{
			outStates->states[ outStates->numStates ] = trans->destNum;
			outStates->numStates++;
			epsClosureRecurse( nfa, trans->destNum, outStates );
		}
	}
}
		
/* Return must be freed with nfaDestroyData */
static FAstateinfo *epsClosure( Nfa nfa, int state )
{
	FAstateinfo *reachable;

	reachable = malloc( sizeof( *reachable ) );
	/* We are guaranteed to have less states than the nfa contains obviously */
	reachable->states = malloc( sizeof( *reachable->states ) * nfa->numStates );

	reachable->states[0] = state;
	reachable->numStates = 1;

	epsClosureRecurse( nfa, state, reachable );
	
	return reachable;
}

static FAstateinfo *epsClosureSet( Nfa nfa, FAstateinfo *states )
{
	FAstateinfo *reachable;
	int i;

	reachable = malloc( sizeof( *reachable ) );
	/* We are guaranteed to have less states than the nfa contains obviously */
	reachable->states = malloc( sizeof( *reachable->states ) * nfa->numStates );

	for( i = 0; i < states->numStates; i++ )
	{
		reachable->states[i] = states->states[i];
	}
	reachable->numStates = states->numStates;

	/* This here needs to be states->numStates rather than reachable->numStates because epsClosureRecurse can change the value of reachable->numStates */
	for( i = 0; i < states->numStates; i++ )
	{
		epsClosureRecurse( nfa, reachable->states[i], reachable );
	}
	
	return reachable;
}


static FAstateinfo *move( Nfa nfa, FAstateinfo *states, int symbol )
{
	FAstateinfo *reachable;
	int i, j;

	reachable = malloc( sizeof( *reachable ) );
	/* We are guaranteed to have less states than the nfa contains obviously */
	reachable->states = malloc( sizeof( *reachable->states ) * nfa->numStates );
	reachable->numStates = 0;

	for( i = 0; i < states->numStates; i++ )
	{
		FAstate *curState;
		int numTrans;

		curState = &nfa->states[ states->states[i] ];
		numTrans = getLinkListCount( curState->transitions );
		for( j = 0; j < numTrans; j++ )
		{
			FAtransition *trans;

			trans = getLinkListItem( curState->transitions, j );
			/* We need to properly transition out of our ALL state */
			if( trans->symbol == ALL )
			{
				addState( reachable, trans->destNum );
			}	
			else if( ALL < trans->symbol && trans->symbol < ALL + NUMSYMBOLS  )
			{
				if( symbol != trans->symbol - ALL )
				{
					addState( reachable, trans->destNum );
				}
			}
			else if( trans->symbol == symbol )
			{
				addState( reachable, trans->destNum );
			}
			if( trans->symbol == symbol )
			{
				addState( reachable, trans->destNum );
			}
		}
	}

	return reachable;
}

/* Checks all the FAstateinfos inside stateList for a match to candidate. Order of states does not matter. If found findFAstate returns the index into the list of the state. If not found it will return -1 */
static int findFAState( FAstateinfo *candidate, FAstateinfo **stateList, int numStates )
{
	int candProd, listProd;
	int i, j, k;
	int index = -1;
	Bool found = false;

	/* Calculate product of states in candidate */
	candProd = 1;
	for( i = 0; i < candidate->numStates; i++ )
	{
		candProd *= (candidate->states[i] + 1 );
	}

	i = 0;
	while( i < numStates && found == false )
	{
		/* Check that they have same number of states as preliminary check */
		if( candidate->numStates == stateList[i]->numStates )
		{
			/* Calculate product and compare as a preliminary check */
			listProd = 1;
			for( j = 0; j < stateList[i]->numStates; j++ )
			{
				listProd *= (stateList[i]->states[j] + 1);
			}
			if( listProd == candProd )
			{
				Bool equal = true;
				/* Our two preliminary checks have worked out, check them element wise now */
				j = 0;
				while( j < stateList[i]->numStates && equal == true )
				{
					Bool present = false;

					k = 0;
					while( k < candidate->numStates && present == false )
					{
						if( stateList[i]->states[j] == candidate->states[k] )
						{
							/* Match has been found for this element, move on to next */
							present = true;
						}
						k++;
					}
					if( present == false )
					{
						/* No match found for element, must be not equal */
						equal = false;
					}
					j++;
				}

				if( equal == false )
				{
					found = false;
				}
				else
				{
					found = true;
					index = i;
				}
			}
		}
		i++;
	}

	return index;
}


static Bool doesContainFinish( Nfa nfa, FAstateinfo *states )
{
	Bool contains = false;
	int i;

	i = 0;
	while( i < states->numStates && contains == false )
	{
		if( nfa->states[states->states[i]].finish )
		{
			contains = true;
		}
		i++;
	}

	return contains;
}

/* Assumes that the NFA has only one start state. If it has more than one then it just uses the first start state and the DFA will likely not be a very useful recogniser  */
Dfa nfaToDfa( Nfa nfa )
{
	Dfa dfa;
	/* This structure will only be used once so I don't really want to add it to my header */
	struct { FAstateinfo **states; int numStates; int capacity; int *marked; int numMarked; } newStates;
	FAstateinfo *start;
	int i, j;
	int *alphabet;

	dfa = nfaCreateEmpty();

	newStates.capacity = DEFCAPACITY;
	newStates.numStates = 0;
	newStates.numMarked = 0;
	newStates.states = malloc( sizeof( *newStates.states ) * newStates.capacity );
	newStates.marked = malloc( sizeof( *newStates.marked ) * newStates.capacity );
	memset( newStates.marked, UNMARKED, sizeof( *newStates.marked ) * newStates.capacity );

	alphabet = nfaGetAlphabet( nfa ); 

	start = nfaGetStartStates( nfa );
	assert( start->numStates > 0 );

	/* Add the start state to our list of states and to our DFA */
	newStates.states[0] = epsClosure( nfa, start->states[0] );
	newStates.numStates++;
	nfaAddEmptyState( dfa, true, doesContainFinish( nfa, newStates.states[0] ) );
	while( newStates.numMarked != newStates.numStates )
	{
		FAstateinfo *curState;

		/* Find next unmarked state and mark it */
		i = 0;
		while( newStates.marked[i] == MARKED )
		{
			i++;
		}
		newStates.marked[i] = MARKED;
		newStates.numMarked++;

		curState = newStates.states[i];
		
		/* This is technically a bounded loop it just doesn't look like one */
		for( j = 0; alphabet[j] != '\0'; j++ )
		{
			FAstateinfo *moves, *closure;
			int foundState;
			Bool isAll;

			/* compute e-closure(move(T,alphabet[j])) */
			moves = move( nfa, curState, alphabet[j] );
			closure = epsClosureSet( nfa, moves );

			if( closure->numStates != 0 )
			{			
				/* Check to see if state is already present */
				foundState = findFAState( closure, newStates.states, newStates.numStates );
				if( foundState == -1 )
				{
					/* Check if our array is big enough to hold. If not then realloc */
					if( newStates.capacity == newStates.numStates )
					{
						FAstateinfo **statesReplace;
						int *markedReplace;

						statesReplace = malloc( sizeof( *statesReplace ) * newStates.capacity * 2 );
						markedReplace = malloc( sizeof( *markedReplace ) * newStates.capacity * 2 );
						memset( markedReplace, UNMARKED, sizeof( *markedReplace ) * newStates.capacity * 2 );
						
						memcpy( statesReplace, newStates.states, newStates.numStates * sizeof( *statesReplace ) );
						memcpy( markedReplace, newStates.marked, newStates.numStates * sizeof( *markedReplace ) );
						free( newStates.states );
						free( newStates.marked );
						newStates.states = statesReplace;
						newStates.marked = markedReplace;
						newStates.capacity *= 2;
					}
					/* Add state */
					newStates.states[newStates.numStates] = closure;
					foundState = nfaAddEmptyState( dfa, false, doesContainFinish( nfa, closure ) );
					newStates.numStates++;
				}
				else
				{
					nfaDestroyData( closure );
				}
				nfaDestroyData( moves );

				/* Make transitions for state. This is a dirty
				 * hack required to facilitate efficient transitions
				 * off of the 'ALL' symbol. The ALL symbol needs
				 * to be at the end of the transition list so 
				 * that it is transitioned from last */
				isAll = ( ALL <= alphabet[j] && alphabet[j] < ALL + NUMSYMBOLS );
				nfaAddTransition( dfa, i, foundState, alphabet[j], isAll ); 
			}
			else
			{
				nfaDestroyData( closure );
				nfaDestroyData( moves );
			}
		}
	}

	nfaDestroyData( start );

	free( alphabet );

	for( i = 0; i < newStates.numStates; i++ )
	{
		nfaDestroyData( newStates.states[i] );
	}
	free( newStates.marked );

	return dfa;
}



Bool nfaSimulateDfa( Dfa dfa, char *string )
{
	FAstate *curState;	
	FAstateinfo *starts;
	int i;
	Bool accept = true;

	starts = nfaGetStartStates( dfa );
	assert( starts->numStates > 0 );
	curState = &dfa->states[starts->states[0]];
	nfaDestroyData( starts );

	while( *string != '\0' && accept == true )
	{
		int numTrans;
		Bool transitioned = false;

		numTrans = getLinkListCount( curState->transitions );
		i =  0;
		while( i < numTrans && !transitioned )
		{
			FAtransition *fat;

			fat = getLinkListItem( curState->transitions, i );
			if( isValidSymbol( *string ) )
			{
				if( fat->symbol == ALL )
				{
					curState = &dfa->states[fat->destNum];
					transitioned = true;
				}	
				else if( ALL < fat->symbol && fat->symbol < ALL + NUMSYMBOLS  )
				{
					if( *string != fat->symbol - ALL )
					{
						curState = &dfa->states[fat->destNum];
						transitioned = true;
					}
				}
				else if( fat->symbol == *string )
				{
					curState = &dfa->states[fat->destNum];
					transitioned = true;
				}
			}
			i++;
		}
		if( !transitioned )
		{
			accept = false;
		}
		string++;
	}

	if( accept && !curState->finish )
	{
		accept = false;
	}

	return accept;
}



int nfaAcceptLongestDfa( Dfa dfa, int (*nextChar)( void ) )
{
	FAstate *curState;	
	FAstateinfo *starts;
	int i;
	Bool notrans = true;
	int acceptChars, numChars;
	int curChar;

	starts = nfaGetStartStates( dfa );
	assert( starts->numStates > 0 );
	curState = &dfa->states[starts->states[0]];
	nfaDestroyData( starts );

	acceptChars = 0; numChars = 1;

	curChar = nextChar();
	notrans = false;
	/* We don't transition off of EOF so no need to check for it here */
	while( notrans == false )
	{
		int numTrans;
		Bool transitioned = false;

		if( curState->finish )
		{
			acceptChars = numChars - 1;
		}

		numTrans = getLinkListCount( curState->transitions );
		i =  0;
		while( i < numTrans && !transitioned )
		{
			FAtransition *fat;

			fat = getLinkListItem( curState->transitions, i );
			if( isValidSymbol( curChar ) )
			{
				if( fat->symbol == ALL )
				{
					curState = &dfa->states[fat->destNum];
					transitioned = true;
				}	
				else if( ALL < fat->symbol && fat->symbol < ALL + NUMSYMBOLS  )
				{
					if( curChar != fat->symbol - ALL )
					{
						curState = &dfa->states[fat->destNum];
						transitioned = true;
					}
				}
				else if( fat->symbol == curChar )
				{
					curState = &dfa->states[fat->destNum];
					transitioned = true;
				}
			}
			i++;
		}
		if( !transitioned )
		{
			notrans = true;
		}
		curChar = nextChar();
		numChars++;
	}


	return acceptChars;
}


Bool isValidSymbol( int symbol )
{
	Bool valid = false;

	if( 0 <= symbol && symbol < NUMSYMBOLS )
	{
		valid = true;
	}

	return valid;
}


void nfaDestroyDfa( Dfa dfa )
{
	nfaDestroyNfa( dfa );
}


void DBGPRINTNFA( Nfa nfa )
{
 	int i, j;

	for( i = 0; i < nfa->numStates; i++ )
	{
		printf( "State Number: %d\n\tStart State: %s\n\tFinish State: %s\n", i, nfa->states[i].start == true ? "Yes" : "No", nfa->states[i].finish == true ? "Yes" : "No" );
		printf("Transitions:\n" );
		for( j = 0; j < getLinkListCount( nfa->states[i].transitions ); j++ )
		{
			FAtransition *fat;
			fat = getLinkListItem( nfa->states[i].transitions, j );
			printf("\t\tTransition: %d\n\t\tSymbol: %d (%c)\n\t\tDestination: State %d\n", j, fat->symbol, fat->symbol, fat->destNum );
		}
		printf("\n\n" );
	}
}	

