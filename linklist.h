#ifndef LINKLIST_H
#define LINKLIST_H

#include <stdlib.h>
#include <string.h>

typedef struct Lnode
{
	void *data;
	struct Lnode *next;
} Lnode;


typedef struct 
{
	Lnode *head;
	int count;
} *Llist;

Llist createLinkList( void );
void destroyLinkList( Llist list );

void insertLinkListItem( Llist list, void *data, int position );
void insertStartLinkListItem( Llist list, void *data );
void insertEndLinkListItem( Llist list, void *data );

void * getLinkListItem( Llist list, int position );
void *getStartLinkListItem( Llist list );
void *getEndLinkListItem( Llist list );

int getLinkListCount( Llist list );

void *removeLinkListItem( Llist list, int position );

#endif
