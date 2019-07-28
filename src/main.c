#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "main.h"
#include "random.h"
#include "tree.h"
#include "scan.h"

void init_option(option *option)
{
	option->width = 1000;
	option->height = 1000;
	option->sample_size = 200000;
	option->min = 10;
	option->max = 20;
	option->thread_num = 0;
	option->algo = al_rand;
	option->r_type = buddhabrot;
	option->scale = 2;
	option->x_offset = 0;
	option->y_offset = 0;
	option->gamma = 0;
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

err long_parse_number(char *str, long *number, int min)
{
	char	*end;
	long	nbr;

	if (*str == '\0')
		return KO;
	nbr = (int)strtol(str, &end, 0);
	if (*end != '\0' || nbr < min)
		return KO;
	*number = nbr;
	return OK;
}

err parse_number(char *str, int *number, int min)
{
	char	*end;
	int		nbr;

	if (str == NULL || *str == '\0')
		return KO;
	nbr = (int)strtol(str, &end, 0);
	if (*end != '\0' || nbr < min)
		return KO;
	*number = nbr;
	return OK;
}

err parse_double(char *str, double *number)
{
	char	*end;
	double	nbr;

	printf("parsing str as double : \"%s\"\n", str);

	if (str == NULL || *str == '\0')
		return KO;
	nbr = strtod(str, &end);
	if (*end != '\0')
		return KO;
	*number = nbr;
	return OK;
}

err parse_algo(char *str, algo *algo)
{
	if (strcmp("random", str) == 0)
	{
		*algo = al_rand;
	}
	else if (strcmp("tree", str) == 0)
	{
		*algo = al_tree;
	}
	else if (strcmp("scan", str) == 0)
	{
		*algo = al_scan;
	}
	return OK;
}

err parse_render(char *str, render_type *render_type)
{	
	if (strcmp("binary", str) == 0)
	{
		*render_type = binary;
	}
	else if (strcmp("layered", str) == 0)
	{
		*render_type = layered;
	}
	else if (strcmp("buddhabrot", str) == 0)
	{
		*render_type = buddhabrot;
	}
	return OK;
}

err parse_args(option *option, int argc, char **argv)
{
	int opt;
	err err;
	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:s:m:M:t:a:r:x:y:z:g:")) != -1)
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
				err = long_parse_number(optarg, &option->sample_size, 1);
				break;
			case 'm':
				err = parse_number(optarg, &option->min, 0);
				break;
			case 'M':
				err = parse_number(optarg, &option->max, 1);
				break;
			case 't':
				err = parse_number(optarg, &option->thread_num, 0);
				break;
			case 'a':
				err = parse_algo(optarg, &option->algo);
				break;
			case 'r':
				err = parse_render(optarg, &option->r_type);
				break;
			case 'x':
				err = parse_double(optarg, &option->x_offset);
				break;
			case 'y':
				err = parse_double(optarg, &option->y_offset);
				option->y_offset = -option->y_offset;
				break;
			case 'z':
				err = parse_double(optarg, &option->scale);
				break;
			case 'g':
				if (strcmp(optarg, "off") == 0)
				{
					option->gamma = 0;
				}
				else if (strcmp(optarg, "auto") == 0)
				{
					option->gamma = -1;
				}
				else
				{
					err = parse_double(optarg, &option->gamma);
				}
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-a random|tree|scan] [-w nbr] [-h nbr] [-s nbr] [-m nbr] [-M nbr] [-t nbr]\n", argv[0]);
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

/*
** Wait for every threads to reach this function
** If the thread is the last to call the function, return true.
** Else, return false.
*/
void threads_sync(data *d)
{
	pthread_mutex_lock(&d->mut);
	++d->sync_count;
	if (d->sync_count == d->option.thread_num)
	{
		pthread_mutex_unlock(&d->mut);
		pthread_mutex_unlock(&d->sync_mut);
	}
	else
	{
		pthread_mutex_unlock(&d->mut);
		pthread_mutex_lock(&d->sync_mut);
		pthread_mutex_lock(&d->mut);
		--d->sync_count;
		if (d->sync_count)
			pthread_mutex_unlock(&d->sync_mut);
		pthread_mutex_unlock(&d->mut);
	}
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
	data.sync_count = 0;
	pthread_mutex_init(&data.mut, NULL);
	pthread_mutex_init(&data.sync_mut, NULL);
	pthread_mutex_lock(&data.sync_mut);

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

	data.view = create_view(data.option.width, data.option.height);
	set_view_position(data.view, data.option.scale, data.option.x_offset, data.option.y_offset);
	if (data.view == NULL)
	{
		printf("Can't allocate memory for the view. Abort\n");
		return (EXIT_FAILURE);
	}
	data.view->render_type = data.option.r_type;
	data.view->gamma = data.option.gamma;

	printf("gamma value : %lf\n", data.option.gamma);

	// calling the algorithm that populate the view
	switch (data.option.algo)
	{
		case al_rand:
			algo_random(&data);
			break;

		case al_tree:
			algo_tree(&data);
			break;

		case al_scan:
			algo_scan(&data);
			break;
	}
 
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