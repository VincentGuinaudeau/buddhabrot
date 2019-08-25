#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "main.h"
#include "parser.h"
#include "random.h"
#include "tree.h"
#include "scan.h"
#include "metro_hast.h"

char *usage =
	"Usage: %s [OPTIONS]\n"
	"OPTIONS :\n"
	"\t-h : display this help and exit."
	"\n--About the image--\n"
	"\t-v binary|layered|buddhabrot : (View) The type of info to render. Default is buddhabrot.\n"
	"\t-w NATURAL -h NATURAL : (Width, Height) The dimension of the final image. Default is 1024 for both.\n"
	"\t-o COMPLEX : (Offset) where to center the render. Default is 0+0i.\n"
	"\t-z DECIMAL : (Zoom) the slice of the complex plan to show. use smaller number to zoom in. Default is 2.\n"
	"\t-g off|auto|DECIMAL : (Gamma) value for the gamma correction. off to skip, auto to let the program find a value. Default is off."
	"\n--About the rendering\n"
	"\t-a random|scan : (Algorithm) the algorithm used to find relevant points. Default is random.\n"
	"\t-t NATURAL : (Threads) number of thread to launch, 0 for auto."
	"\t-s NATURAL : (Sample) for random algorithm, the number of points to find. Default is 200.000 .\n"
	"\t-m NATURAL -M NATURAL : (Minimum, Maxium) the range of steps a point must escape in dto be included in the render. Default is 10 and 20."
	"\n--About the fractal\n"
	"\t-F mandelbrot|julia : (Fractal) The kind of function to iterate on. Default is mandelbrot.\n"
	"\t-p DECIMAL : (Power) The value for the power parameter. Default is 2.\n"
	"\t-f COMPLEX : (Factor) The value for the factor parameter. Default is 1+0i.\n"
	"\t-j COMPLEX : (Julia) The additionnal parameter needed for the Julia function. Default is 0+0i.\n"
	"TYPES :\n"
	"\tNATURAL : A positive integer number.\n"
	"\tDECIMAL : A positive floating number.\n"
	"\tCOMPLEX : A complex number.\n"
	"";

void init_option(option *option)
{
	option->width = 1024;
	option->height = 1024;
	option->sample_size = 200000;
	option->min = 10;
	option->max = 20;
	option->thread_num = 0;
	option->algo = al_rand;
	option->r_type = buddhabrot;
	option->scale = 2;
	option->offset = (complex){.r = 0, .i = 0};
	option->gamma = 0;
	option->f_params.power = 2;
	option->f_params.type = mandelbrot;
	option->f_params.point = (complex){.r = 0, .i = 0};
	option->f_params.factor = (complex){.r = 1, .i = 0};
}

err parse_args(option *option, int argc, char **argv)
{
	int opt;
	err err;

	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:s:m:M:t:a:v:o:z:g:F:p:f:j:")) != -1)
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
			{
				char *algo_values[] = {"random", "scan", "tree", "metrohast"};
				err = parse_enum(optarg, (int*)&option->algo, algo_values, 4);
				break;	
			}
			case 'v':
			{
				char *render_values[] = {"binary", "layered", "buddhabrot"};
				err = parse_enum(optarg, (int*)&option->r_type, render_values, 3);
				break;
			}
			case 'o':
				err = parse_complex(optarg, &option->offset);
				option->offset.i = -option->offset.i;
				break;
			case 'z':
				err = parse_double(optarg, &option->scale);
				break;
			case 'g':
			{
				char *gamma_values[] = {"off", "auto"};
				err = parse_enum(optarg, (int*)&option->gamma, gamma_values, 2);
				if (err == KO)
				{
					err = parse_double(optarg, &option->gamma);
				}
				else if (option->gamma == 1)
				{
					option->gamma = -1;
				}
				break;
			}
			case 'F':
			{
				char *fractal_values[] = {"mandelbrot", "julia"};
				err = parse_enum(optarg, (int*)&option->f_params.type, fractal_values, 2);
				break;
			}
			case 'p':
				err = parse_double(optarg, &option->f_params.power);
				break;
			case 'f':
				err = parse_complex(optarg, &option->f_params.factor);
				break;
			case 'j':
				err = parse_complex(optarg, &option->f_params.point);
				break;
			default: /* '?' */
				fprintf(stderr, usage, argv[0]);
				return (KO);
		}
		if (err == KO)
		{
			fprintf(stderr, "option '%c' is invalid\n", opt);
			return (KO);
		}
	}
	return (OK);
}

void display_progress(data *d)
{
	int nbr = (long)d->found * 10000 / d->option.sample_size;
	if (d->progress < nbr)
	{
		d->progress = nbr;
		// printf("\r%d.%d%d%%", d->progress / 100, d->progress / 10 % 10, d->progress % 10);
		printf("\r%d.%d%d%% (tested : %ld / correct : %ld = %f)", d->progress / 100, d->progress / 10 % 10, d->progress % 10, d->tested, d->found, (float)d->tested / (float)d->found);
		fflush(stdout);
	}
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
	data.tested = 0;
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
	set_view_position(data.view, data.option.scale, &data.option.offset);
	if (data.view == NULL)
	{
		printf("Can't allocate memory for the view. Abort\n");
		return (EXIT_FAILURE);
	}
	data.view->render_type = data.option.r_type;
	data.view->gamma = data.option.gamma;

	printf("gamma value : %lf\n", data.option.gamma);

	// calling the algorithm that populate the view
	err algo_ok = KO;
	switch (data.option.algo)
	{
		case al_rand:
			algo_ok = algo_random(&data);
			break;

		case al_scan:
			algo_ok = algo_scan(&data);
			break;

		case al_tree:
			algo_ok = algo_tree(&data);
			break;

		case al_metro_hast:
			algo_ok = algo_metro_hast(&data);
			break;
	}

	if (algo_ok == OK)
	{
		printf("writing to disk\n");
		err res = write_view_to_disk(data.view, path);
		if (res == KO)
		{
			printf("Can't open file '%s' : %s. Abort.\n", path, strerror(errno));
			return (EXIT_FAILURE);
		}
	}
	free(data.view);
	free(data.threads);

	return (EXIT_SUCCESS);
}