#include "tree.h"
#include "math.h"

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
	n->status = NODE_OPEN;

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

	if (result == -2)
	{
		n->status = NODE_COMPLETED;
		free_tree(n);
	}
	return (result);
}

/*
** explore down from node for SIMILARE_LEVEL_TO_COMPLETE levels.
** may close the node if the conditions are met.
** If there is no node left to explore, we 
*/
err step_explore_tree(data *d)
{
	data_tree *dtree = d->arg;

	// lock a node to compute. If there is no node,
	// this mean we can go to the render step.
	
	pthread_mutex_lock(&data->mut);
		elem *elem = dtree->todo;
		node_ext *ext = (node_ext)elem->data;
		dtree->todo = snap_list(dtree->todo);
		ADD_STATUS(((node_ext)elem->data)->base, NODE_LOCKED);
	pthread_mutex_unlock($data->mut);

	// call compute_node
	compute_node(d,
		         ext->base,
		         &ext->pos,
		         ext->level,
		         SIMILARE_LEVEL_TO_COMPLETE);

	// insert new open node into the todo


	// unlock the node
		
}

void *thread_main_tree(data *d)
{
	node *tree = d->arg;

}

err algo_tree(data *d)
{
	// creating the todo
	data_tree *dtree;
	dtree = malloc(sizeof(data_tree));
	if (dtree == NULL)
	{
		return (KO);
	}

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
	node_ext = malloc(sizeof(node_ext));
	if (ext == NULL)
	{
		return (KO);
	}
	ext->tree = dtree->tree;
	ext->level = 0;
	ext->pos->r = 0;
	ext->pos->i = 0;
	dtree->todo = create_new_elem(ext);
	if (dtree->todo == NULL)
	{
		return (KO);
	}

	// TODO : launch threads

	return (OK);
}