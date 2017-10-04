#include "include/main.h"

#include <unistd.h>

void		init_option(t_option *option)
{
}

err			parse_number(char* str, int *number)
{
	char	*end;
	int		nbr;

	if (*str == '\0')
		return (KO);
	nbr = (int)strtol(str, &end, 0);
	if (*end != '\0')
		return (KO);
	*number = nbr;
	return (OK);
}

err			parse_args(t_option *option, int argc, char **argv)
{
	int		opt;
	init_option(option);
	while ((opt = getopt(argc, argv, "w:h:p:m:M:")) != -1) {
		switch (opt)
		{
			case 'w':
				break;
			case 'h':
				break;
			case 'p':
				break;
			case 'm':
				break;
			case 'M':
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s\n", argv[0]);
				return (KO);
		}
	}

}

int			main(int argc, char** argv)
{
	t_option	option;

	parse_args(&option, argc, argv);
}