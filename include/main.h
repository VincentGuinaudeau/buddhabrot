#ifndef MAIN_H_
#define MAIN_H_

typedef int err;
enum {KO, OK};

typedef int bool;
enum {false, true};

typedef enum e_algo {
	al_rand,
	al_tree,
	al_scan
} algo;

#include <pthread.h>
#include <string.h>

#include "view.h"

typedef struct s_option
{
	int			width;
	int			height;
	long		sample_size;
	int			min;
	int			max;
	int			thread_num;
	algo     	algo;
	render_type	r_type;
	double		scale;
	double		x_offset;
	double		y_offset;
	double		gamma;
} option;

typedef struct s_data
{
	option			option;
	view			*view;
	void			*arg;
	pthread_mutex_t mut;
	long			sync_count;
	pthread_mutex_t sync_mut;
	pthread_t		*threads;
	int				found;
	int				progress;
} data;

err launch_threads(data *d, void*(*func)(data*));
void threads_sync(data *d);

#endif /* MAIN_H_ */