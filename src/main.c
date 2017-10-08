#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "math.h"
#include "view.h"

void init_option(t_option *option)
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

void default_option(t_option *option)
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

err parse_args(t_option *option, int argc, char **argv)
{
	int opt;
	err err;
	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:s:m:M:")) != -1)
	{
		switch (opt)
		{
			case 'w':
				err = parse_number(optarg, &(option->width));
				break;
			case 'h':
				err = parse_number(optarg, &(option->height));
				break;
			case 's':
				err = parse_number(optarg, &(option->sample_size));
				break;
			case 'm':
				err = parse_number(optarg, &(option->min));
				break;
			case 'M':
				err = parse_number(optarg, &(option->max));
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

int main(int argc, char** argv)
{
	t_option option;
	char *path = "./test.pgm";

	if (parse_args(&option, argc, argv) == KO)
		return (EXIT_FAILURE);

	printf("allocating memory.\n");
	view *view = create_view(option.width, option.height);
	if (view == NULL)
	{
		printf("Can't allocate memory for the view. Abort\n");
		return (EXIT_FAILURE);
	}

    complex c;
	int found = 0;
	int progress = 0;
	int nbr;

    srand (time(NULL));
	printf("staring loop. sample_size : %d points\n", option.sample_size);
	while (found < option.sample_size)
	{
		c.r = (double)rand() / RAND_MAX * 4.0 - 2.0;
		c.i = (double)rand() / RAND_MAX * 4.0 - 2.0;
		nbr = number_of_step_to_escape(&c, option.max);
		if (nbr >= option.min && nbr <= option.max)
		{
			add_point_to_view(view, &c);
			found++;
			nbr = (long)found * 10000 / option.sample_size;
			if (progress < nbr)
			{
				progress = nbr;
				printf("\r%d.%d%d%%", progress / 100, progress / 10 % 10, progress % 10);
			}
		}
	}

	printf("\nwriting to disk\n");
	err res = write_view_to_disk(view, path);
	if (res == KO)
	{
		printf("Can't open file '%s' : %s. Abort.\n", path, strerror(errno));
		return (EXIT_FAILURE);
	}
	free(view);

	printf("done\n");
	return (EXIT_SUCCESS);
}