// List functions
/* Adapted from: Yu Zhang (yuzhang@ustc.edu.cn) */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "list.h"

static ListItr glistitr = NULL;

// new an error or a warn message
listNode
newlistNode(void *item)
{
	listNode new;
	NEW0(new);
	new->item = item;
	return new;
}

List
newList()
{
	List new;
	NEW0(new);
	listNode newNode;
	NEW0(newNode);
	new -> first = newNode;
	new -> last = newNode;
	return new;
}

/**
 *  Links element item as first element of the list
 */
static void
linkFirst(List list, void *item)
{
	listNode first = list->first;
	listNode firstNext = first->next;
	listNode newnode = newlistNode(item);
	newnode->next = firstNext;
	newnode->prev = first;
	newnode->list = list;
	if ( firstNext == NULL )
		list->last = newnode;
	else
		firstNext->prev = newnode;
	first -> next = newnode;
	list->size ++;
}

/**
 *  Links element item as first element of the list
 */
static void
linkLast(List list, void *item)
{
	listNode last = list->last;
	listNode newnode = newlistNode(item);
	newnode->prev = last;
	newnode->next = NULL;
	newnode->list = list;
	last->next = newnode;
	list->last = newnode;
	list->size ++;
}

/**
 *  Inserts element item before non-NULL & non-default-first node succ
 */
static void
linkBefore(List list, void *item, listNode succ)
{
	assert(succ != NULL);
	listNode pred = succ->prev;
	listNode newnode = newlistNode(item);
	newnode->next = succ;
	newnode->prev = succ->prev;
	newnode->list = list;
	succ->prev = newnode;
	pred->next = newnode;
	list->size ++;
}

/**
 *  Unlinks non-NULL & non-default-first node 
 */
static void *
unlink(List list, listNode node)
{
	assert(node != NULL);
	listNode 	prev, next;
	void 	*item;

	item = node->item;
	prev = node->prev;
	next = node->next;
	if (prev == NULL)
		list->first = next;
	else {
		prev->next = next;
		node->prev = NULL;
	}
	if (next == NULL)
		list->last = prev;
	else {
		next->prev = prev;
		node->next = NULL;
	}

	node->item = NULL;
	free(node);
	list->size--;
	return item;
}

/**
 *  Returns the first element in this list
 */
void *
getFirst(List list)
{
	listNode first = list->first->next;
	if ( first == NULL ) {
		printf("The list is empty!\n");
		exit(-1);
	}
	return first->item;
}

/**
 *  Returns the last element in this list
 */
void *
getLast(List list)
{
	listNode last = list->last;
	if ( last == NULL || list->first->next == NULL) {
		printf("The list is empty!\n");
		exit(-1);
	}
	return last->item;
}

/**
 *  Removes and returns the first element from this list
 */
void *
removeFirst(List list)
{
	listNode first = list->first->next;
	if ( first == NULL ) {
		printf("The list is empty!\n");
		exit(-1);
	}
	return unlink(list, first);
}

/**
 *  Removes and returns the last element from this list
 */
void *
removeLast(List list)
{
	listNode last = list->last;
	if ( last == NULL || list->first->next == NULL ) {
		printf("The list is empty!\n");
		exit(-1);
	}
	return unlink(list, last);
}

/**
 * Inserts the specified element at the beginning of the list
 */
void
addFirst(List list, void * item)
{
	linkFirst(list, item);
}

/**
 * Inserts the specified element to the end of the list
 */
void
addLast(List list, void * item)
{
	linkLast(list, item);
}

/**
 * Returns the index of the first occurence of the specified 
 * element in the list, or -1 if the list does not contain
 * the element.
 */
int
indexof(List list, void *item)
{
	int index = 0;
	listNode node = list->first->next;
	while (node != NULL) {
		if ( (node->item == item) )
			return index;
		index ++;
		node = node->next;
	}
	return -1;
}

/**
 * Returns TRUE if the list contains the specified element.
 */
bool
listcontains(List list, void *item) 
{
	return indexof(list, item) != -1;
}

/**
 * Returns the number of elements in the list
 */
int
listsize(List list)
{
	return list->size;
}

/**
 * Appends the specified element to the end of the list
 */
bool
listaddItem(List list, void * item)
{
	linkLast(list, item);
	return TRUE;
}

/**
 * Removes the first occurrence of the specified element from
 * the list, if it is present.
 */
bool
listremoveItem(List list, void *item)
{
	listNode node = list->first->next;
	while(node!=NULL) {
		if (node->item == item) {
			unlink(list, node);
			return TRUE;
		}
		node = node->next;
	}
	return FALSE;
}

/**
 * Removes all of the elements from the list
 */
void
listclear(List list, void destroyItem())
{
	listNode node = list->first->next;
	while (node != NULL) {
		listNode next = node->next;
		if (destroyItem != NULL)
			destroyItem(&node->item);
		node->item = NULL;
		free(node);
		node = next;
	}
	list->size = 0;
}

/**
 * Returns the node at the specified element index
 */
listNode
listnode(List list, int index)
{
	listNode node;
	int i;
	if ( index < (list->size >> 1)) {
		node = list->first->next;
		for ( i = 0; i < index; i++)
			node = node->next;
		return node;
	} else {
		node = list->last;
		for ( i = list->size - 1; i > index; i--)
			node = node->prev;
		return node;
	}
}

/**
 * Returns the element at the specified position in the list
 */
void *
listget(List list, int index)
{
	listNode node = listnode(list, index);
	return node==NULL ? NULL : node->item;
}

/**
 * Replaces the element at the specified position in the list
 */
void *
listset(List list, int index, void *item)
{
	listNode node = listnode(list, index);
	if ( node==NULL ) {
		printf("Index %d is out of the bound!\n", index);
		exit(-1);
	}
	void *oldval = node->item;
	node->item = item;	
	return oldval;
}

/**
 * Inserts the specified element at the specified position in the 
 * list
 */
void 
listadd(List list, int index, void *item)
{
	if ( index == list->size ) {
		linkLast(list, item);
		return;
	}
	listNode node = listnode(list, index);
	if ( node==NULL ) {
		printf("Index %d is out of the bound!\n", index);
		exit(-1);
	}
	linkBefore(list, item, node);
}

/**
 * Removes the element at the specified position in the list
 */
void *
listremove(List list, int index)
{
	listNode node = listnode(list, index);
	if ( node==NULL ) {
		printf("Index %d is out of the bound!\n", index);
		exit(-1);
	}
	return unlink(list, node);
}

/**
 * Destroys the list
 */
void
destroyList(List *list, void destroyItem())
{
	listclear(*list, destroyItem);
	free((*list)->first);
	free(*list);
	*list = NULL;
}
/*************************************************************
 * Functions for iterating elements
 ************************************************************/

/**
 * Creates an iterator
 */
ListItr
newListItr(List list, int index)
{
	ListItr new;
	NEW0(new);
	new->list = list;
	new->next = listnode(list, index);
	new->nextIndex = index;
	return new;
}

/**
 * Gets the singleton global List iterator
 */
ListItr
getGListItr(List list, int index)
{
	if ( glistitr == NULL )
	NEW0(glistitr);
	glistitr->list = list;
	glistitr->next = listnode(list, index);
	glistitr->nextIndex = index;
	return glistitr;
}

/**
 * Resets the list iterator
 */
void
resetListItr(ListItr itr, List list, int index)
{
	assert(itr != NULL);
	itr->list = list;
	itr->next = listnode(list, index);
	itr->nextIndex = index;
}

/**
 * Returns TRUE if there is next element
 */
bool
hasNext(ListItr itr)
{
	return itr->nextIndex < itr->list->size;
}

/**
 * Advance and returns the next element
 */
void *
nextItem(ListItr itr)
{
	if ( !hasNext(itr) ) {
		printf("Already reach the end of the list.\n");
		return NULL;
	}
	itr->lastRet = itr->next;
	itr->next = itr->next->next;
	itr->nextIndex ++;
	return itr->lastRet->item;
}

/**
 * Returns TRUE if there is previous element
 */
bool
hasPrevious(ListItr itr)
{
	return itr->nextIndex > 0;
}

/**
 * Backward and returns the previous element
 */
void *
prevItem(ListItr itr)
{
	if ( !hasPrevious(itr) ) {
		printf("Already reach the beginning of the list.\n");
		return NULL;
	}
	if ( itr->next == NULL )
		itr->next = itr->list->last;
	else
		itr->next = itr->next->prev;
	
	itr->lastRet = itr->next;
	itr->nextIndex --;
	return itr->lastRet->item;
}

/**
 * Destroys the iterator
 */
void
destroyListItr(ListItr *itr)
{
	free(*itr);
	*itr = NULL;
}

/**
 * Return next iterator
 */
ListItr nextItr (ListItr itr)
{
	if ( !hasNext(itr) ) {
		printf("Already reach the end of the list.\n");
		return NULL;
	}
	ListItr newItr = newListItr(itr->list, itr->nextIndex+1);
	return newItr;
}


/**
 * Return previous iterator
 */
ListItr prevItr (ListItr itr)
{
	if ( !hasPrevious(itr) ) {
		printf("Already reach the beginning of the list.\n");
		return NULL;
	}
	ListItr newItr = newListItr(itr->list, itr->nextIndex-11);
	return newItr;
}


/**
 * destroyItem example.  
 */
void destroyItem(void* item) {
    free(item);
}


/**
 * iterList example.
 */
void iterList (ListItr itr, void hook())
{
	while(hasNext(itr))
	{
		// what you want to do
		void* item = nextItem(itr);  //会自动往下走的
		hook(item);
	}
}


void removeNode(listNode node)
{
	// 仅适用于prev一定存在的情况
	listNode prev = node -> prev;
	listNode next = node -> next;
	prev -> next = next;
	if(next != NULL) next -> prev = prev;
	node -> item = NULL;
	node -> prev = NULL;
	node -> next = NULL;
	((List)(node -> list)) -> size --;
	free(node);
}

listNode getFirstNode(List list)
{
	return list->first->next;
}