#include <stdio.h>
#include <stdlib.h>
#include "main.h"

void *thread_main_random(data *d)
{
	int nbr;
	trace	*trace = malloc(sizeof(trace) + sizeof(complex) * (d->option.max + 2));

	pthread_mutex_lock(&d->mut);
	view	*view = clone_view(d->view);
	pthread_mutex_unlock(&d->mut);
	if (view == NULL || trace == NULL)
	{
		printf("Can't allocate memory for the thread. Abort thread.\n");
		return ((void*)EXIT_FAILURE);
	}

	while (d->found < d->option.sample_size)
	{
		trace->points[0].r = (double)rand() / RAND_MAX * 4.0 - 2.0;
		trace->points[0].i = (double)rand() / RAND_MAX * 4.0 - 2.0;
		compute_trace(trace, d->option.max);
		if (
			trace->length >= d->option.min &&
			trace->length <= d->option.max
		)
		{
			add_trace_to_view(view, trace);
			++d->found;
			nbr = (long)d->found * 10000 / d->option.sample_size;
			if (d->progress < nbr)
			{
				d->progress = nbr;
				printf("\r%d.%d%d%%", d->progress / 100, d->progress / 10 % 10, d->progress % 10);
				fflush(stdout);
			}
		}
	}

	pthread_mutex_lock(&d->mut);
	merge_view(d->view, view);
	pthread_mutex_unlock(&d->mut);

	free(view);
	free(trace);

	return (EXIT_SUCCESS);
}

err algo_random(data *d)
{
	printf("retreiving points.\n");
	printf("sample size : %ld points.\n", d->option.sample_size);
	printf("precision of random : %d discret values betwen -2 and 2.\n", RAND_MAX);

	srand(time(NULL));

	printf("computing:\n0.00%%");
	fflush(stdout);
	launch_threads(d, thread_main_random);
	printf("\n");

	return (OK);
}