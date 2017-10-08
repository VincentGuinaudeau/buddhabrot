#ifndef MAIN_H_
#define MAIN_H_

typedef int err;
enum {KO, OK};

typedef int bool;
enum {false, true};

#include <pthread.h>

#include "view.h"

typedef struct s_option
{
	int		width;
	int		height;
	int		sample_size;
	int		min;
	int		max;
	double	scale;
	double	x_offset;
	double	y_offset;
} option;

typedef struct s_data
{
	option			option;
	view			*view;
	pthread_mutex_t mut;
	pthread_t		*threads;
	int				found;
	int				progress;
} data;

#endif /* MAIN_H_ */