#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "tree.h"

void free_tree(node *n)
{
	if (n->leafs != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			free_tree(&(n->leafs[i]));
		}
		free(n->leafs);
		n->leafs = NULL;
	}
}

/*
** move the complex down to one level in the direction provided
*/
void move_down(complex *c, int move, int level)
{
	double step = (double)1 / (double)(1 << (level - 1));
	c->r += step * (IS_BOT(move) ? -1 : 1);
	c->i += step * (IS_LEFT(move) ? -1 : 1);
}

int compute_node_rec(data *d, node *n, complex *c, int level, int remaining)
{
	complex buff;
	int reduce;
	int value;

	n->leafs = NULL;
	n->nbr_step = number_of_step_to_escape(c, d->option.max);
	n->status = NODE_NO_STATUS;

	//printf("level:%d\tx:%lf\ty:%lf\tstep:%d\n", level, c->r, c->i, n->nbr_step);

	reduce = n->nbr_step;

	if (remaining)
	{
		n->leafs = malloc(sizeof(node) * 4);
		if (n->leafs == NULL)
		{
			return -1;
		}
		for (int i = 0; i < 4; i++)
		{
			buff.r = c->r;
			buff.i = c->i;
			move_down(&buff, i, level + 1);
			value = compute_node_rec(d,
			                         n->leafs + i,
			                         &buff,
			                         level + 1,
			                         remaining - 1);
			if (reduce != value)
			{
				reduce = -2;
			}
		}
	}
	return (reduce);
}

/*
** compute the node and some of it's children.
** if the node and all the children it compute have the same nbr_step,
** all of the children are freed and the node is marked closed
*/
int compute_node(data *d, node *n, complex *c, int level, int remaining)
{
	int result = compute_node_rec(d, n, c, level, remaining);
	if (level > 1000)
	{
		result = -2;
	}

	if (result > -2)
	{
		printf("%02d level %d\n", result, level);
		n->status = NODE_COMPLETED;
		free_tree(n);
	}
	return (result);
}

/*
** collect all open node recursivly into a list
*/
elem *list_open_node(data *d, node *n, complex *c, int level)
{
	if (HAS_STATUS(n, NODE_COMPLETED))
	{
		return (NULL);
	}
	elem *list = NULL;
	if (n->leafs == NULL)
	{
		node_ext *ext;
		ext = malloc(sizeof(node_ext));
		if (ext == NULL)
		{
			// TODO : handle the failed malloc
			return (NULL);
		}
		ext->node = n;
		ext->level = level;
		ext->pos = *c;
		list = create_new_elem(ext);
	}
	else
	{
		complex pos;
		elem *buff;
		for (int i = 0; i < 4; i++)
		{
			pos.r = c->r;
			pos.i = c->i;
			move_down(&pos, i, level + 1);
			buff = list_open_node(d, n->leafs + i, &pos, level + 1);
			if (buff != NULL)
			{
				if (list != NULL)
				{
					insert_list_at_start(buff, list);
				}
				list = buff;
			}
		}
	}
	return (list);
}

/*
** explore down from node for SIMILARE_LEVEL_TO_COMPLETE levels.
** may close the node if the conditions are met.
** If there is no node left to explore, we 
*/
err step_explore_tree(data *d)
{
	data_tree *dtree = d->arg;

	/*
	** lock a node to compute. If there is no node,
	** this mean we can go to the render step.
	*/
	pthread_mutex_lock(&d->mut);
	if (dtree->list == NULL)
	{
		pthread_mutex_unlock(&d->mut);
		return (KO);
	}
	elem *open_node = dtree->list;
	// remove the first elem from the list
	dtree->list = snap_list(dtree->list);

	++d->found;

	// if the new list is empty, we keep the mutex locked
	// so that the other thread have to wait.
	if (dtree->list != NULL)
	{
		pthread_mutex_unlock(&d->mut);
	}

	node_ext *ext = (node_ext*)open_node->data;
	free(open_node);
	ADD_STATUS(ext->node, NODE_LOCKED);

	/*
	** call compute_node
	*/
	int result = compute_node(
		d,
		ext->node,
		&ext->pos,
		ext->level,
		SIMILARE_LEVEL_TO_COMPLETE);
	if (result == -1)
	{
		return (KO);
	}

	/*
	** insert new open node into the list
	*/
	elem *open_list;
	open_list = list_open_node(
		d,
		ext->node,
		&ext->pos,
		ext->level);
	if (open_list != NULL)
	{
		if (dtree->list != NULL)
		{
			pthread_mutex_lock(&d->mut);
			insert_list_at_start(open_list, dtree->list);
		}
		dtree->list = open_list;

		pthread_mutex_unlock(&d->mut);
	}
	else if (dtree->list == NULL)
	{
		pthread_mutex_unlock(&d->mut);
	}

	/*
	** unlock the node
	*/
	REMOVE_STATUS(ext->node, NODE_LOCKED);
	free(ext);

	return (OK);
}

void *thread_main_tree(data *d)
{
	while (step_explore_tree(d) == OK);

	return (EXIT_SUCCESS);
}

err algo_tree(data *d)
{
	// allocating the data for the tree algorithm
	data_tree *dtree;
	dtree = malloc(sizeof(data_tree));
	if (dtree == NULL)
	{
		return (KO);
	}
	d->arg = dtree;

	// creating the tree
	dtree->tree = malloc(sizeof(node));
	if (dtree->tree == NULL)
	{
		return (KO);
	}
	dtree->tree->leafs = NULL;
	dtree->tree->nbr_step = d->option.max + 1;
	dtree->tree->status = NODE_NO_STATUS;

	// creating the todo
	node_ext *ext;
	ext = malloc(sizeof(node_ext));
	if (ext == NULL)
	{
		return (KO);
	}
	ext->node = dtree->tree;
	ext->level = 0;
	ext->pos.r = 0;
	ext->pos.i = 0;
	dtree->list = create_new_elem(ext);
	if (dtree->list == NULL)
	{
		return (KO);
	}

	launch_threads(d, thread_main_tree);

	printf("%d steps\n", d->found);

	return (OK);
}