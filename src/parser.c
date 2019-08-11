#include <string.h>
#include <stdlib.h>

#include "parser.h"

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

	if (str == NULL || *str == '\0')
		return KO;
	nbr = strtod(str, &end);
	if (*end != '\0')
		return KO;
	*number = nbr;
	return OK;
}

/**
 * Parse a complex number from a string, using strtod to parse the two floating points number parts
 * The string must have one of those form :
 *   "[NUM]"  (only the real part)
 *   "[NUM]i" (only the imaginary part)
 *   "[NUM][+|- NUM]i" or "[NUM]i[+|- NUM]" (both real and imaginary part)
 * where NUM is a valid number for strtod.
 */
err parse_complex(char *str, complex *c)
{
	double real = 0;
	double imag = 0;
	double value = 0;
	bool has_i = false;
	char *cursor = str;
	char *endptr = NULL;

	value = strtod(cursor, &endptr);
	if (cursor == endptr)
		return KO;
	cursor = endptr;
	if (*cursor == 'i')
	{
		has_i = true;
		cursor++;
		imag = value;
	}
	else
	{
		real = value;
	}

	if (*cursor != '\0')
	{
		value = strtod(cursor, &endptr);
		if (cursor == endptr)
			return KO;
		cursor = endptr;
		if ((*cursor == 'i') == has_i)
			return KO;

		if (*cursor == 'i')
		{
			cursor++;
			imag = value;
		}
		else
		{
			real = value;
		}

		if (*cursor != '\0')
			return KO;
	}

	c->r = real;
	c->i = imag;
	return OK;
}

err parse_enum(char *str, int *result, char **values, int array_length)
{
	for (int i = 0; i < array_length; i++)
	{
		if (strcmp(str, values[i]) == 0)
		{
			*result = i;
			return OK;
		}
	}
	return KO;
}