#include <stdio.h>
#include <stdlib.h>
#include "fract.h"
#define BATCH_SIZE 8192
#define SUBSAMPLING 1

void *thread_main_scan(data *d)
{
	int     nbr;
	long    point;
	long    stop;
	trace	*trace = malloc(sizeof(trace) + sizeof(complex) * (d->option.max + 3));

	pthread_mutex_lock(&d->mut);
	view	*view = clone_view(d->view);
	pthread_mutex_unlock(&d->mut);
	if (view == NULL || trace == NULL)
	{
		printf("Can't allocate memory for the thread. Abort thread.\n");
		return ((void*)EXIT_FAILURE);
	}

	double  substep = view->step / SUBSAMPLING;

	while (1)
	{
		pthread_mutex_lock(&d->mut);
		point = d->sync_count;
		if (d->sync_count >= d->option.sample_size)
		{
			pthread_mutex_unlock(&d->mut);
			break;
		}
		stop = point + BATCH_SIZE;
		d->sync_count = stop;
		pthread_mutex_unlock(&d->mut);
		stop = stop < d->option.sample_size ? stop : d->option.sample_size;

		for (long i = point; i < stop; i++)
		{
			for (int j = 0; j < SUBSAMPLING * SUBSAMPLING; j++)
			{
				init_trace(
					&d->option.f_params,
					trace,
					(i % view->x - view->x / 2) * view->step + view->offset.r + (j % SUBSAMPLING + 0.5) * substep,
					(i / view->x - view->y / 2) * view->step + view->offset.i + (j % SUBSAMPLING / 0.5) * substep
				);
				compute_trace(&d->option.f_params, trace, d->option.max);
				if (
					trace->length >= d->option.min &&
					trace->length <= d->option.max
				)
				{
					add_trace_to_view(view, &d->option.f_params, trace);
				}
				nbr = point * 10000 / d->option.sample_size;
				if (d->progress < nbr)
				{
					d->progress = nbr;
					printf("\r%d.%d%d%%", d->progress / 100, d->progress / 10 % 10, d->progress % 10);
					fflush(stdout);
				}
			}
		}
	}

	pthread_mutex_lock(&d->mut);
	merge_view(d->view, view);
	pthread_mutex_unlock(&d->mut);

	free(trace);
	free(view);

	return (EXIT_SUCCESS);
}

err algo_scan(data *d)
{
	d->option.sample_size = d->view->x * d->view->y;
	printf("number of points : %ld points.\n", d->option.sample_size);

	// set_view_position(d->view, 0.25, -0.8, 0.2);

	launch_threads(d, thread_main_scan);
	printf("\n");

	return (OK);
}