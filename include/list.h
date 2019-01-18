#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>

/*
** a simple implementation of a 2-way linked list
** it does not support :
**  * shared elem (shared data is ok).
**  * circularity
*/

typedef struct s_elem
{
	void		    *data;
	struct s_elem	*next;
	struct s_elem	*prev;
}   elem;

#define RETURN_IF_NULL(ptr) if (ptr == NULL) return NULL;
#define GOTO_END(list)   while (list->next != NULL) list = list->next;
#define GOTO_START(list) while (list->prev != NULL) list = list->prev;

elem *create_new_elem(void *data);
elem *add_new_elem_to_list(elem *list, void *data);
void insert_list(elem *list, elem *store);
void link_list(elem *list, elem *store);
elem *snap_list(elem *list);

#endif /* LIST_H_ */