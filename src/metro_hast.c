#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"

#define USE_MTWISTER 1

#if USE_MTWISTER == 1
	#include "mtwister.h"
	static mt_rand global_mt_state;
	#define RANDOM(state) (mt_gen_rand(state))
#else
	#define RANDOM() ((double)rand() / RAND_MAX)
#endif

void mutate(mt_rand *local_mt_state, double zoom, complex *c, complex *result)
{
	if (RANDOM(local_mt_state) * 5 < 4)
	{
		double rand_module = RANDOM(local_mt_state) * (1 / zoom) * 0.000001;
		double rand_angle  = RANDOM(local_mt_state) * M_PI * 2;
		result->r = c->r + rand_module * cos(rand_angle);
		result->i = c->i + rand_module * sin(rand_angle);
	}
	else
	{
		c->r = RANDOM(local_mt_state) * 4.0 - 2.0;
		c->i = RANDOM(local_mt_state) * 4.0 - 2.0;
	}
}



void *thread_main_metro_hast(data *d)
{
	int contribution;
	int previous_contribution = 1;
	// int previous_trace_length = 1;
	double acceptance_probability;
	double current_ratio = 1;
	double candidate_ratio;
	complex candidate;
	complex current;
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


	current.r = RANDOM(&local_mt_state) * 4.0 - 2.0;
	current.i = RANDOM(&local_mt_state) * 4.0 - 2.0;

	while (d->found < d->option.sample_size)
	{
		d->tested++;
		mutate(&local_mt_state, view->scale, &current, &candidate);
		init_trace(&d->option.f_params, trace, candidate.r, candidate.i);
		compute_trace(&d->option.f_params, trace, d->option.max);
		if (
			trace->length >= d->option.min &&
			trace->length <= d->option.max
		)
		{
			contribution = add_trace_to_view(view, &d->option.f_params, trace);
			if (contribution > 0)
			{
				candidate_ratio = (1.f - (d->option.max - trace->length) / d->option.max);
				acceptance_probability = exp(
					log(contribution * (candidate_ratio / current_ratio)) -
					log(previous_contribution * (current_ratio / candidate_ratio))
				);

				if (acceptance_probability >= 1 || acceptance_probability > RANDOM(&local_mt_state))
				{
					previous_contribution = contribution;
					// previous_trace_length = trace->length;
					current_ratio = candidate_ratio;
					current = candidate;

					++d->found;
					display_progress(d);					
				}
			}
		}
	}

	// combining all the images
	pthread_mutex_lock(&d->mut);
	merge_view(d->view, view);
	pthread_mutex_unlock(&d->mut);

	// clearing the memory
	free(view);
	free(trace);

	return (EXIT_SUCCESS);
}

err algo_metro_hast(data *d)
{
	printf("retreiving points.\n");
	printf("sample size : %ld points.\n", d->option.sample_size);

	#if USE_MTWISTER == 1
		global_mt_state = mt_seed_rand(time(NULL));
	#else
		srand(time(NULL));
	#endif

	printf("computing:\n0.00%%");
	fflush(stdout);
	launch_threads(d, thread_main_metro_hast);
	printf("\n");

	return (OK);
}