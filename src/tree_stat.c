#include <stdlib.h>
#include <stdio.h>
#include "tree.h"

err init_tree_stat(tree_stat *stat)
{
	stat->nodes = 0;
	stat->nodes_in_range = 0;
	stat->levels_count = malloc((MAX_DEPTH + 1) * sizeof(int));
	if (stat->levels_count == NULL)
	{
		printf("Can't allocate memory for a stat objet. Abort\n");
		return (KO);
	}

	int i = 0;
	while (i < MAX_DEPTH + 1)
	{
		stat->levels_count[i] = 0;	
		++i;
	}

	return (OK);
}

/*
** add source's stat value to the destination's values,
** then reset the source stat's object
*/
void thread_sync_stat(tree_stat *source, tree_stat *destination)
{
	destination->nodes          += source->nodes;
	destination->nodes_in_range += source->nodes_in_range;
	source->nodes = 0;
	source->nodes_in_range = 0;
	int i = 0;
	while (i < MAX_DEPTH + 1)
	{
		destination->levels_count[i] += source->levels_count[i];
		source->levels_count[i] = 0;
		++i;
	}
}

int compute_depth_target(data *d, tree_stat *stat)
{
	int i = 0;
	while (i < MAX_DEPTH + 1)
	{
		if (stat->levels_count[i] > d->option.sample_size)
		{
			return i;
		}
		++i;
	}
	return MAX_DEPTH;
}

void free_tree_stat(tree_stat *stat)
{
	free(stat->levels_count);
}