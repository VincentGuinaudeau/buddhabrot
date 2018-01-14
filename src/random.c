#include <stdio.h>
#include <stdlib.h>
#include "main.h"

void *thread_main_random(data *data)
{
    complex c;
	int nbr;
	int *buffer = malloc(sizeof(int) * (data->option.max + 2));
	if (buffer == NULL)
	{
		printf("Can't allocate memory for a buffer. Abort thread.\n");
		return ((void*)EXIT_FAILURE);
	}

	while (data->found < data->option.sample_size)
	{
		c.r = (double)rand() / RAND_MAX * 4.0 - 2.0;
		c.i = (double)rand() / RAND_MAX * 4.0 - 2.0;
		nbr = number_of_step_to_escape(&c, data->option.max);
		if (nbr >= data->option.min && nbr <= data->option.max)
		{
			prepare_point_for_view(data->view, buffer, &c, data->option.max);
			pthread_mutex_lock(&data->mut);
				add_computed_point_to_view(data->view, buffer);
				++data->found;
				nbr = (long)data->found * 10000 / data->option.sample_size;
				if (data->progress < nbr)
				{
					data->progress = nbr;
					printf("\r%d.%d%d%%", data->progress / 100, data->progress / 10 % 10, data->progress % 10);
					fflush(stdout);
				}
			pthread_mutex_unlock(&data->mut);
		}
	}
	free(buffer);
	return (EXIT_SUCCESS);
}

err algo_random(data *d)
{
	printf("retreiving points.\n");
	printf("sample size : %d points.\n", d->option.sample_size);
	printf("precision of random : %d discret values betwen -2 and 2.\n", RAND_MAX);

	srand(time(NULL));

	printf("computing:\n0.00%%");
	fflush(stdout);
	launch_threads(d, thread_main_random);
	printf("\n");

	return (OK);
}