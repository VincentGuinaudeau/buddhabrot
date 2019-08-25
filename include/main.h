#ifndef MAIN_H_
#define MAIN_H_

#include "definitions.h"

typedef enum e_algo {
	al_rand,
	al_scan,
	al_tree,
	al_metro_hast
} algo;

#include <pthread.h>
#include <string.h>

#include "fract.h"
#include "view.h"

typedef struct s_option
{
	int				width;
	int				height;
	long			sample_size;
	int				min;
	int				max;
	int				thread_num;
	algo     		algo;
	render_type		r_type;
	double			scale;
	complex			offset;
	double			gamma;
	fract_params	f_params;
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
	long			tested;
	long			found;
	int				progress;
} data;

void display_progress(data *d);
err launch_threads(data *d, void*(*func)(data*));
void threads_sync(data *d);

#endif /* MAIN_H_ */