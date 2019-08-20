#include <stdio.h>
#include <stdlib.h>
#include "main.h"

#define USE_MTWISTER 1

#if USE_MTWISTER == 1
#include "mtwister.h"
static mt_rand global_mt_state;
#endif

void *thread_main_random(data *d)
{
	int nbr;
	trace	*trace = malloc(sizeof(trace) + sizeof(complex) * (d->option.max + 3));

	pthread_mutex_lock(&d->mut);
	view	*view = clone_view(d->view);
	#if USE_MTWISTER == 1
		mt_rand local_mt_state = mt_seed_rand(mt_gen_rand_long(&global_mt_state));
	#endif
	pthread_mutex_unlock(&d->mut);
	if (view == NULL || trace == NULL)
	{
		printf("Can't allocate memory for the thread. Abort thread.\n");
		return ((void*)EXIT_FAILURE);
	}

	while (d->found < d->option.sample_size)
	{
		init_trace(
			&d->option.f_params,
			trace,
			#if USE_MTWISTER == 1
				mt_gen_rand(&local_mt_state) * 4.0 - 2.0,
				mt_gen_rand(&local_mt_state) * 4.0 - 2.0
			#else
				(double)rand() / RAND_MAX * 4.0 - 2.0,
				(double)rand() / RAND_MAX * 4.0 - 2.0
			#endif
		);
		// printf("test : %lf %lf\n", );
		compute_trace(&d->option.f_params, trace, d->option.max);
		if (
			trace->length >= d->option.min &&
			trace->length <= d->option.max
		)
		{
			add_trace_to_view(view, &d->option.f_params, trace);
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

	#if USE_MTWISTER == 1
		global_mt_state = mt_seed_rand(time(NULL));
	#else
		srand(time(NULL));
	#endif

	printf("computing:\n0.00%%");
	fflush(stdout);
	launch_threads(d, thread_main_random);
	printf("\n");

	return (OK);
}