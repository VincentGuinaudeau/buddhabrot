#include "list.h"

/*
** create a new eleement
** return NULL if malloc can't allocate.
*/
elem *create_new_elem(void *data)
{
	elem *new = malloc(sizeof(elem));
	RETURN_IF_NULL(new);
	new->data = data;
	new->next = NULL;
	new->prev = NULL;
	return new;
}

/*
** create a new element and ad it to the end of the list
** return NULL if malloc can't alloc.
*/
elem *add_new_elem_to_list(elem *list, void *data)
{
	elem *new = create_new_elem(data);
	RETURN_IF_NULL(new);
	GOTO_END(list);
	list->next = new;
	return new;
}

/*
** insert a list into another (list into store).
** the cut is made between the elem given and its next elem.
*/
void insert_list(elem *list, elem *store)
{
	elem *next = store->next;
	store->next = list;
	list->prev = store;
	GOTO_END(list);
	list->next = next;
	next->prev = list;
}

/*
** link two list together.
** the link is made between the end od store and the start of list
*/
void insert_list_at_end(elem *list, elem *store)
{
	GOTO_START(list);
	GOTO_END(store);
	store->next = list;
	list->prev = store;
}

/*
** link two list together.
** the link is mad between the end of list and the start of store
*/
void insert_list_at_start(elem *list, elem *store)
{
	GOTO_START(store);
	GOTO_END(list);
	list->next = store;
	store->prev = list;
}

/*
** cut the list into two.
** the cut is made between the elem given and its next elem
** the function return a pointer to the next elem.
*/
elem *snap_list(elem *list)
{
	RETURN_IF_NULL(list->next)
	elem *next = list->next;
	list->next = NULL;
	next->prev = NULL;
	return next;
}

/*
** remove the given element from is list and return it.
** the given elem is untouched, only the surrounding elem are modified.
** this leave the possibility to fetch the list
** from the pointer of the extracted element.
*/
elem *remove_from_list(elem *list)
{
	if (list->next != NULL)
	{
		list->next->prev = list->prev;
	}
	if (list->prev != NULL)
	{
		list->prev->next = list->next;
	}
	return list;
}
