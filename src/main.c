#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "tree.h"
#include "main.h"
#include "random.h"

void init_option(option *option)
{
	option->width = 1000;
	option->height = 1000;
	option->sample_size = 200000;
	option->min = 10;
	option->max = 20;
	option->thread_num = 0;
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

err parse_number(char* str, int *number, int min)
{
	char	*end;
	int		nbr;

	if (*str == '\0')
		return (KO);
	nbr = (int)strtol(str, &end, 0);
	if (*end != '\0' || nbr < min)
		return (KO);
	*number = nbr;
	return (OK);
}

err parse_args(option *option, int argc, char **argv)
{
	int opt;
	err err;
	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:s:m:M:t:")) != -1)
	{
		switch (opt)
		{
			case 'w':
				err = parse_number(optarg, &option->width, 1);
				break;
			case 'h':
				err = parse_number(optarg, &option->height, 1);
				break;
			case 's':
				err = parse_number(optarg, &option->sample_size, 1);
				break;
			case 'm':
				err = parse_number(optarg, &option->min, 1);
				break;
			case 'M':
				err = parse_number(optarg, &option->max, 1);
				break;
			case 't':
				err = parse_number(optarg, &option->thread_num, 0);
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-w nbr] [-h nbr] [-s nbr] [-m nbr] [-M nbr] [-t nbr]\n", argv[0]);
				return (KO);
		}
		if (err == KO)
		{
			fprintf(stderr, "option '%c' only accept a positive number\n", opt);
			return (KO);
		}
	}
	//default_option(option);
	return (OK);
}


err launch_threads(data *d, void*(*func)(data*))
{
	d->threads = malloc(sizeof(pthread_t) * d->option.thread_num);
	if (d->threads == NULL)
	{
		printf("Can't allocate memory for the threads. Abort\n");
		return (KO);
	}

	for (int i = 0; i < d->option.thread_num; i++)
	{
		pthread_create(d->threads + i, NULL, (void *(*)(void *))func, d);
	}
	for (int i = 0; i < d->option.thread_num; i++)
	{
		pthread_join(d->threads[i], NULL);
	}
	return (OK);
}

int main(int argc, char **argv)
{
	data data;
	data.found = 0;
	data.progress = 0;
	pthread_mutex_init(&data.mut, NULL);

	if (parse_args(&(data.option), argc, argv) == KO)
		return (EXIT_FAILURE);

	printf("number of threads (%s) : ", data.option.thread_num == 0 ? "auto" : "manual");
	if (data.option.thread_num == 0)
	{
		data.option.thread_num = get_nprocs();
	}
	printf("%d\n", data.option.thread_num);

	// todo : option for path
	char *path = "./out.pgm";

	printf("allocating memory.\n");		
	data.view = create_view(data.option.width, data.option.height);
	// set_view_position(data.view, 1.5, -0.5, 0);
	if (data.view == NULL)
	{
		printf("Can't allocate memory for the view. Abort\n");
		return (EXIT_FAILURE);
	}

	// calling the algorithm that populate the view
	algo_random(&data);
	// algo_tree(&data);

	printf("writing to disk\n");
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