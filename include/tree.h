#ifndef TREE_H_
#define TREE_H_

#include "main.h"
#include "math.h"
#include "list.h"

enum
{
	top_right,
	top_left,
	bot_right,
	bot_left
};

typedef enum e_tree_step
{
	EXPLORATION,
	RENDERING
}	tree_step;

#define IS_TOP(move)   (~move & 2)
#define IS_BOT(move)    (move & 2)
#define IS_LEFT(move)   (move & 1)
#define IS_RIGHT(move) (~move & 1)

// status
#define NODE_NO_STATUS   0
#define NODE_COMPLETED   (1 << 0) // the node doesn't need further eploration
#define NODE_CLOSED      (1 << 1) // the algorithm as choose to no explore this node further
#define NODE_USEFUL      (1 << 2) // some leafs of this node are useful
#define NODE_BEHIND      (1 << 3) // this node is behind the range
#define NODE_IN_RANGE    (1 << 4) // this node is in range
#define NODE_OVER        (1 << 5) // this node is over the range

#define ADD_STATUS(node, stat)    (node->status |= stat)
#define REMOVE_STATUS(node, stat) (node->status &= ~stat)
#define HAS_STATUS(node, stat)    (node->status & stat)

/*
** number of level to consider to compute if
** the node need further exploration
*/
#define SIMILARE_LEVEL_TO_COMPLETE 4
#define MAX_LEVEL_DOWN_PER_PASS 7

/*
** The position of the node in the complex plane
** will be deduced by it's position in the tree
*/
typedef struct s_node
{
	struct s_node	*leafs;
	struct s_node	*root;
	short			nbr_step;
	unsigned char	status;
}	node;

/*
** A structure storing a node alonside informations
** to avoid having to find it in the tree to retreive its position.
*/
typedef struct s_node_ext
{
	node			*node;
	complex			pos;
	unsigned short	level;
}	node_ext;

typedef struct s_tree_thread
{
	long			nodes;
	long			nodes_in_range;
	int				*buffer;
}	tree_thread;

typedef struct s_data_tree
{
	node			*tree;
	elem			*list;
	pthread_mutex_t mut; // read / write on the task list
	tree_step		step; // current step
	long			nodes;
	long			nodes_in_range;
}	data_tree;

err algo_tree(data *d);

#endif /* TREE_H_ */