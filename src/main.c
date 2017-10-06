#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

void		init_option(t_option *option)
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

void		default_option(t_option *option)
{
	if (option->width == -1)
		option->width = -1;
	if (option->height == -1)
		option->height = -1;
	if (option->sample_size == -1)
		option->sample_size = -1;
	if (option->min == -1)
		option->min = -1;
	if (option->max == -1)
		option->max = -1;
}

err			parse_number(char* str, int *number)
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

err			parse_args(t_option *option, int argc, char **argv)
{
	int		opt;
	err		err;
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
		if (err)
		{
			fprintf(stderr, "option '%c' only accept a positive number\n", opt);
			return (KO);
		}
	}
	default_option(option);
	return (OK);
}

int			main(int argc, char** argv)
{
	t_option	option;

	if (parse_args(&option, argc, argv))
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}