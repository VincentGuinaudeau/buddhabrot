#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h> // for debug
#include "tree.h"

/*
** free all childrens of the node, but not the node itself
*/
void free_tree(node *n)
{
	if (n->leafs != NULL)
	{
		for (int i = 0; i < 4; i++)
			free_tree(&(n->leafs[i]));
		free(n->leafs);
		n->leafs = NULL;
	}
}

err insert_node_in_list(elem **open_list, node_ext *ext)
{
	node_ext *new_ext;
	new_ext = malloc(sizeof(node_ext));
	if (new_ext == NULL)
		return (KO);
	new_ext->node = ext->node;
	new_ext->pos = ext->pos;
	new_ext->level = ext->level;
	elem *new_open = create_new_elem(new_ext);
	if (new_open == NULL)
		return (KO);
	if (*open_list != NULL)
		insert_list_at_start(new_open, *open_list);
	*open_list = new_open;
	return (OK);
}

err init_open_list(data_tree *dtree)
{
	node_ext *ext;
	ext = malloc(sizeof(node_ext));
	if (ext == NULL)
		return (KO);
	ext->node = dtree->tree;
	ext->level = 0;
	ext->pos.r = 0;
	ext->pos.i = 0;
	dtree->list = NULL;
	return insert_node_in_list(&dtree->list, ext);
}

/*
** move the complex down to one level in the direction provided
*/
void move_down(complex *c, int move, int level)
{
	double step = 2;
	while (level)
	{
		--level;
		step /= 2;
	}
	c->r += step * (IS_BOT(move) ? -1 : 1);
	c->i += step * (IS_LEFT(move) ? -1 : 1);
}

void mark_node_useful(node *n)
{
	while (n != NULL)
	{
		if (HAS_STATUS(n, NODE_USEFUL))
		{
			return;
		}
		ADD_STATUS(n, NODE_USEFUL);
		n = n->root;
	}
}

/*
** determine if a node need further exploration.
** If it does not, the function return 0.
** If it does, the function return the number of levels
** you can explore before calling it again.
*/
int should_continue_exploration(data *d, node *n, short level)
{
	int min = n->nbr_step;
	int max = n->nbr_step;
	int steps = SIMILARE_LEVEL_TO_COMPLETE;

	// if (level > 18)
	// 	return 0;
	if (level > 13)
	{
		return (0);
	}

	while (steps && n->root != NULL)
	{
		n = n->root;
		if (min > n->nbr_step)
			min = n->nbr_step;
		if (max < n->nbr_step)
			max = n->nbr_step;
		if (max - min > 1)
			break;
		--steps;
	}
	return steps;
}

/*
** compute the node.
** if remaining > 0, we allocate and compute the childrens.
** if next_check == 0, we call should_continue_exploration
**   to test if the node can be closed
*/
err explore_tree(data *d,
                 tree_thread *tthread,
                 elem **open_list, 
                 node_ext *ext,
                 int remaining)
{
	node_ext buff;

	ext->node->leafs = NULL;
	ext->node->nbr_step = number_of_step_to_escape(&ext->pos, d->option.max);
	ext->node->status = NODE_NO_STATUS;

	++tthread->nodes;
	if (ext->node->nbr_step < d->option.min)
	{
		ADD_STATUS(ext->node, NODE_BEHIND);
	}
	else if (ext->node->nbr_step > d->option.max)
	{
		ADD_STATUS(ext->node, NODE_OVER);
	}
	else
	{
		++tthread->nodes_in_range;
		ADD_STATUS(ext->node, NODE_IN_RANGE);
		mark_node_useful(ext->node);
	}

	// printf("%p l %d x %lf y %lf n %d s %d\n", ext->node, ext->level, ext->pos.r, ext->pos.i, ext->node->nbr_step, ext->node->status);

	// check if the node can be closed
	int next_check = should_continue_exploration(d, ext->node, ext->level);
	if (next_check == 0)
	{
		// we close and stop the function here
		ADD_STATUS(ext->node, NODE_COMPLETED);
		return (OK);
	}

	if (remaining) // if remaining is still positive
	{
		// we continue the exploration
		ext->node->leafs = malloc(sizeof(node) * 4);
		if (ext->node->leafs == NULL)
			return (KO);
		for (int i = 0; i < 4; i++)
		{
			ext->node->leafs[i].root = ext->node;
			buff.node = ext->node->leafs + i;
			buff.pos.r = ext->pos.r;
			buff.pos.i = ext->pos.i;
			buff.level = ext->level + 1;
			move_down(&buff.pos, i, ext->level + 1);
			if (explore_tree(d,
			                 tthread,
			                 open_list,
			                 &buff,
			                 remaining - 1) == KO)
				return (KO);
		}
	}
	else
	{
		// else, we add this node in the open_list
		if (insert_node_in_list(open_list, ext) == KO)
			return (KO);
	}
	return (OK);
}

/*
** walk the tree and insert the relevant node into the view.
*/
err render_tree(data *d,
               tree_thread *tthread,
               elem **open_list,
               node_ext *ext,
               int remaining)
{
	node_ext buff;
	// printf("%p l %d x %lf y %lf n %d s %d %d\n", ext->node, ext->level, ext->pos.r, ext->pos.i, ext->node->nbr_step, ext->node->status, HAS_STATUS(ext->node, NODE_USEFUL));
	// if (!HAS_STATUS(ext->node, NODE_USEFUL))
	// 	return (OK);
	if (remaining)
	{
		if (!HAS_STATUS(ext->node, NODE_COMPLETED))
		{
			for (int i = 0; i < 4; i++)
			{
				buff.node = ext->node->leafs + i;
				buff.pos.r = ext->pos.r;
				buff.pos.i = ext->pos.i;
				buff.level = ext->level + 1;
				move_down(&buff.pos, i, ext->level + 1);
				render_tree(d, tthread, open_list, &buff, remaining - 1);
			}
		}
		if (HAS_STATUS(ext->node, NODE_IN_RANGE))
		{
			prepare_point_for_view(d->view, tthread->buffer, &ext->pos, d->option.max);
			// tthread->buffer[1] = -1;
			pthread_mutex_lock(&d->mut);
			add_computed_point_to_view(d->view, tthread->buffer);
			pthread_mutex_unlock(&d->mut);
		}
	}
	else
	{
		if (insert_node_in_list(open_list, ext) == KO)
			return (KO);
	}
	return (OK);
}

/*
** explore down from node for MAX_LEVEL_DOWN_PER_PASS levels.
** may close the node if the conditions are met.
** If there is no node left to explore, we 
*/
err step_explore_tree(data *d,
                      tree_thread *tthread,
                      err(*func)(data*,
                                 tree_thread*,
                                 elem**,
                                 node_ext*,
                                 int))
{
	data_tree *dtree = d->arg;
	bool keep_locked = false;

	/*
	** lock a node to compute. If there is no node,
	** this mean we can go to the render step.
	*/
	pthread_mutex_lock(&dtree->mut);
	if (dtree->list == NULL)
	{
		pthread_mutex_unlock(&dtree->mut);
		return (KO);
	}
	elem *open_node = dtree->list;
	// remove the first elem from the list
	dtree->list = snap_list(dtree->list);

	// if the new list is empty, we keep the mutex locked
	// so that the other thread have to wait.
	if (dtree->list != NULL)
		pthread_mutex_unlock(&dtree->mut);
	else
		keep_locked = true;

	node_ext *ext = (node_ext*)open_node->data;
	free(open_node);

	/*
	** calling the function
	*/
	elem *open_list = NULL;
	int result = func(
		d,
		tthread,
		&open_list,
		ext,
		MAX_LEVEL_DOWN_PER_PASS);
	free(ext);
	if (result == KO)
		return (KO);

	/*
	** insert new open node into the list
	** and lock / unlock the mutex if necessary
	*/
	if (open_list != NULL)
	{
		if (!keep_locked)
			pthread_mutex_lock(&dtree->mut);
		if (dtree->list != NULL)
			insert_list_at_start(open_list, dtree->list);
		dtree->list = open_list;
		pthread_mutex_unlock(&dtree->mut);
	}
	else if (keep_locked)
	{
		pthread_mutex_unlock(&dtree->mut);
	}

	return (OK);
}

void *thread_main_tree(data *d)
{
	data_tree *dtree = d->arg;
	tree_thread tthread;
	tthread.nodes = 0;
	tthread.nodes_in_range = 0;
	tthread.buffer = malloc(sizeof(int) * (d->option.max + 2));
	if (tthread.buffer == NULL)
	{
		printf("Can't allocate memory for a buffer. Abort\n");
		return ((void*)EXIT_FAILURE);
	}

	// tree exploration
	while (step_explore_tree(d, &tthread, &explore_tree) == OK);

	threads_sync(d);

	// next step init
	pthread_mutex_lock(&d->mut);
	if (dtree->step != RENDERING)
	{
		dtree->step = RENDERING;
		printf("%ld nodes calculated\n", dtree->nodes);
		printf("%ld node useful\n", dtree->nodes_in_range);
		printf("printing the tree\n");
		dtree->nodes += tthread.nodes;
		dtree->nodes_in_range += tthread.nodes_in_range;
		init_open_list(dtree);
	}
	pthread_mutex_unlock(&d->mut);

	// tree rendering
	while (step_explore_tree(d, &tthread, &render_tree) == OK);

	free(tthread.buffer);

	return (EXIT_SUCCESS);
}

err algo_tree(data *d)
{
	// allocating the data for the tree algorithm
	data_tree dtree;
	d->arg = &dtree;
	dtree.nodes = 0;
	dtree.nodes_in_range = 0;
	pthread_mutex_init(&dtree.mut, NULL);
	dtree.step = EXPLORATION;

	// creating the tree
	dtree.tree = malloc(sizeof(node));
	if (dtree.tree == NULL)
		return (KO);
	dtree.tree->leafs = NULL;
	dtree.tree->root = NULL;
	// we know the origin is on the mandelbrot set
	dtree.tree->nbr_step = d->option.max + 1;
	dtree.tree->status = NODE_OVER;

	init_open_list(&dtree);

	printf("exploring the tree\n");
	launch_threads(d, thread_main_tree);

	free_tree(dtree.tree);

	return (OK);
}