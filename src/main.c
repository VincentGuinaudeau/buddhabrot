#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "main.h"
#include "math.h"
#include "view.h"

void init_option(option *option)
{
	option->width = -1;
	option->height = -1;
	option->sample_size = -1;
	option->min = -1;
	option->max = -1;
	option->scale = 4;
	option->x_offset = 0;
	option->y_offset = 0;
}

void default_option(option *option)
{
	if (option->width == -1)
		option->width = 1000;
	if (option->height == -1)
		option->height = 1000;
	if (option->sample_size == -1)
		option->sample_size = 200000;
	if (option->min == -1)
		option->min = 10;
	if (option->max == -1)
		option->max = 20;
}

err parse_number(char* str, int *number)
{
	char	*end;
	int		nbr;

	if (*str == '\0')
		return (KO);
	nbr = (int)strtol(str, &end, 0);
	if (*end != '\0' || nbr <= 0)
		return (KO);
	*number = nbr;
	return (OK);
}

err parse_args(option *option, int argc, char **argv)
{
	int opt;
	err err;
	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:s:m:M:")) != -1)
	{
		switch (opt)
		{
			case 'w':
				err = parse_number(optarg, &option->width);
				break;
			case 'h':
				err = parse_number(optarg, &option->height);
				break;
			case 's':
				err = parse_number(optarg, &option->sample_size);
				break;
			case 'm':
				err = parse_number(optarg, &option->min);
				break;
			case 'M':
				err = parse_number(optarg, &option->max);
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-w nbr] [-h nbr] [-s nbr] [-m nbr] [-M nbr]\n", argv[0]);
				return (KO);
		}
		if (err == KO)
		{
			fprintf(stderr, "option '%c' only accept a positive number\n", opt);
			return (KO);
		}
	}
	default_option(option);
	return (OK);
}

void *thread_main(data *data)
{
    complex c;
	int nbr;
	int *buffer = malloc(sizeof(int) * (data->option.max + 1));
	if (buffer == NULL)
	{
		printf("Can't allocate memory for a buffer. Abort thread.");
		return ((void*)EXIT_FAILURE);
	}

	pthread_mutex_lock(&data->mut);
	srand (time(NULL) + data->random_seed);
	data->random_seed += 10;
	pthread_mutex_unlock(&data->mut);
	while (data->found < data->option.sample_size)
	{
		c.r = (double)rand() / RAND_MAX * 4.0 - 2.0;
		c.i = (double)rand() / RAND_MAX * 4.0 - 2.0;
		nbr = number_of_step_to_escape(&c, data->option.max);
		if (nbr >= data->option.min && nbr <= data->option.max)
		{
			prepare_point_for_view(data->view, buffer, &c);
			pthread_mutex_lock(&data->mut);
			add_computed_point_to_view(data->view, buffer);
			data->found++;
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

int main(int argc, char **argv)
{
	data data;
	char *path = "./test.pgm";

	if (parse_args(&(data.option), argc, argv) == KO)
		return (EXIT_FAILURE);

	printf("allocating memory.\n");
	data.view = create_view(data.option.width, data.option.height);
	if (data.view == NULL)
	{
		printf("Can't allocate memory for the view. Abort\n");
		return (EXIT_FAILURE);
	}

	data.random_seed = 0;
	data.found = 0;
	pthread_mutex_init(&data.mut, NULL);
	int threads = get_nprocs();
	data.threads = malloc(sizeof(pthread_t) * threads);
	if (data.threads == NULL)
	{
		printf("Can't allocate memory for the threads. Abort\n");
		return (EXIT_FAILURE);
	}

	printf("retreiving points.\n");
	printf("sample size : %d points.\n", data.option.sample_size);
	printf("number of threads : %d.\n", threads);
	printf("precision of random : %d discret values betwen -2 and 2.\n", RAND_MAX);

	for (int i = 0; i < threads; i++)
	{
		pthread_create(&data.threads[i], NULL, (void*(*)(void*))thread_main, &data);
	}
	void *retval;
	for (int i = 0; i < threads; i++)
	{
		pthread_join(data.threads[i], &retval);
	}

	printf("\nwriting to disk\n");
	err res = write_view_to_disk(data.view, path);
	if (res == KO)
	{
		printf("Can't open file '%s' : %s. Abort.\n", path, strerror(errno));
		return (EXIT_FAILURE);
	}
	free(data.view);
	free(data.threads);

	printf("done\n");
	return (EXIT_SUCCESS);
}