#include "math.h"

double distance_squared(double x, double y)
{
	return x * x + y * y;
}

void calc_step(complex *walker, complex *start_point)
{
	double	buff;

	buff = walker->r * walker->r - walker->i * walker->i + start_point->r;
	walker->i = walker->r * walker->i * 2 + start_point->i;
	walker->r = buff;
}

/*
** Get the number of iteration to espace the circle of radius 2
** If it take more than 'max' step,
** the function stop iterating and return max + 1
*/
int number_of_step_to_escape(complex *point, int max)
{
	int		i = 0;
	complex	walker = {point->r, point->i};

	while (distance_squared(walker.r, walker.i) <= 4 && i <= max)
	{
		calc_step(&walker, point);
		i++;
	}
	return i;
}