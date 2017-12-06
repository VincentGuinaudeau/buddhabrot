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

#define IS_TOP(move)   (~move & 2)
#define IS_BOT(move)    (move & 2)
#define IS_LEFT(move)   (move & 1)
#define IS_RIGHT(move) (~move & 1)

// status
#define NODE_NO_STATUS 0
#define NODE_LOCKED    (1 << 0) // a thread is working on this node
#define NODE_COMPLETED (1 << 1) // the node doesn't need further eploration
#define NODE_USEFUL    (1 << 2) // some leafs of this node are useful

#define ADD_STATUS(node, stat)    (node->status |= stat)
#define REMOVE_STATUS(node, stat) (node->status &= ~stat)
#define HAS_STATUS(node, stat)    (node->status & stat)

/*
** number of level to consider to compute if
** the node need further exploration
*/
#define SIMILARE_LEVEL_TO_COMPLETE 3
#define MAX_LEVEL_DOWN_PER_PASS 5

/*
** The position of the node in the complex plane
** will be deduced by it's position in the tree
*/
typedef struct s_node
{
	struct s_node	*leafs;
	int				nbr_step;
	int				status;
}	node;

/*
** A structure storing a node alonside informations
** to avoid having to find it in the tree to retreive its position.
*/
typedef struct s_node_ext
{
	node			*node;
	int				level;
	int				next_check;
	complex			pos;
}	node_ext;

typedef struct s_data_tree
{
	node *tree;
	elem *list;
}	data_tree;

err algo_tree(data *d);

#endif /* TREE_H_ */