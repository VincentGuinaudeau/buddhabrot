#ifndef TREE_H_
#define TREE_H_

#include "main.h"
#include "math.h"
#include "list.h"

/*
** The enums are just here as an indicator.
** Suffeling the values will not change
** the behaviour of the program.
*/

typedef enum e_diagonal_dir
{
	top_right, // 0
	top_left,  // 1
	bot_right, // 2
	bot_left   // 3
}	diagonal_dir;

typedef enum e_straight_dir
{
	right, // 0
	left,  // 1
	top,   // 2
	bot    // 3
}	straight_dir;

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
#define NODE_NO_STATUS    0
#define NODE_COMPLETED    (1 << 0) // the node doesn't need further eploration
#define NODE_CLOSED       (1 << 1) // the algorithm as choose to no explore this node further
#define NODE_USEFUL       (1 << 2) // some leafs of this node are useful
#define NODE_BEHIND       (1 << 3) // this node is behind the range
#define NODE_IN_RANGE     (1 << 4) // this node is in range
#define NODE_OVER         (1 << 5) // this node is over the range
#define NODE_AS_NEIGHBORS (1 << 6)

#define ADD_STATUS(node, stat)    (node->status |= stat)
#define REMOVE_STATUS(node, stat) (node->status &= ~stat)
#define HAS_STATUS(node, stat)    (node->status & stat)

/*
** number of level to consider to compute if
** the node need further exploration
*/
#define SIMILARE_LEVEL_TO_COMPLETE 6
#define MAX_LEVEL_DOWN_PER_PASS 7

/*
** max depth allowed on the tree
** defined by testing the maximum precison
** for a double in different scenario.
** (labo/test_float.c)
*/
#define MAX_DEPTH 54

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

#define GET_DIAG_DIR_FROM_ROOT(n) (n->root - n)

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

/*
** values produced by each thread and merge periodicaly
*/
typedef struct s_tree_stat
{
	long			nodes;
	long			nodes_in_range;
	int				*levels_count;
}	tree_stat;

/*
** variable each threads uses
*/
typedef struct s_tree_thread
{
	int				*buffer;
	tree_stat		stat;
}	tree_thread;

/*
** global variables of the tree algorithm
*/
typedef struct s_data_tree
{
	node			*tree;
	elem			*list;
	pthread_mutex_t mut;
	tree_step		step; // current step
	tree_stat		stat;
	int				depth_target;
	int				*levels_count;
}	data_tree;

/*
** tree.c
*/
err algo_tree(data *d);

/*
** tree_stat.c
*/
err init_tree_stat(tree_stat *stat);
void thread_sync_stat(tree_stat *source, tree_stat *destination);
int compute_depth_target(data *d, tree_stat *stat);
void free_tree_stat(tree_stat *stat);

#endif /* TREE_H_ */