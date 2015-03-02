#include "linklist.h"


Llist createLinkList( void )
{
	Llist list;

	list = malloc( sizeof( *list ) );

	memset( list, 0, sizeof( *list ) );

	return list;
}

void destroyLinkList( Llist list )
{
	while( list->count > 0 )
	{
		/* This will decrement count for us */
		removeLinkListItem( list, 0 );
	}

	free( list );
}

void insertLinkListItem( Llist list, void *data, int position )
{
	Lnode *node;
	Lnode **curNode;

	node = malloc( sizeof( *node ) );
	node->data = data;
	node->next = NULL;

	if( position < 0 )
	{
		position = 0;
	}
	else if( position > list->count )
	{
		position = list->count;
	}

	if( position == 0 )
	{
		/* special case insert at head */
		node->next = list->head;
		list->head = node;
	}
	else
	{
		curNode = &list->head;
		
		while( position > 1 )
		{
			curNode = &(*curNode)->next;
			position--;
		}
		node->next = (*curNode)->next;
	        (*curNode)->next = node;	
	}

	list->count++;
}	

void insertStartLinkListItem( Llist list, void *data )
{
	insertLinkListItem( list, data, 0 );
}

void insertEndLinkListItem( Llist list, void *data )
{
	insertLinkListItem( list, data, list->count );
}

void *getLinkListItem( Llist list, int position )
{
	Lnode **curNode;

	curNode = &list->head;

	if( position < 0 || position >= list->count )
	{
		return NULL;
	}

	while( position > 0 )
	{
		curNode = &(*curNode)->next;
		position--;
	}

	return (*curNode)->data;
}

void *getStartLinkListItem( Llist list )
{
	return getLinkListItem( list, 0 );
}

void *getEndLinkListItem( Llist list )
{
	return getLinkListItem( list, list->count - 1 );
}

int getLinkListCount( Llist list )
{
	return list->count;
}

void *removeLinkListItem( Llist list, int position )
{
	Lnode **curNode;
	Lnode *freeNode;
	void *data;

	curNode = &list->head;

	if( position < 0 || position >= list->count )
	{
		return NULL;
	}

	while( position > 0 )
	{
		curNode = &(*curNode)->next;
		position--;
	}
	freeNode = (*curNode);
	data = freeNode->data;

	*curNode = (*curNode)->next;

	free( freeNode );
	list->count--;

	return data;
}
