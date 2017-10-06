#include "math.h"

inline void	step(complex *walker, complex *start_point)
{
	double	buff;

	buff = walker->r * walker->r - walker->i * walker->i + c->r;
	walker->i = walker->r * walker->i * 2 + c->i;
	walker->r = buff;
}

/*
** Get the number to espace the circle of radius 2
** If it take more than 'max' step,
** the function stop iterating and return max + 1
*/
int		number_of_step_to_escape(complex *c, int max)
{
	int		i = 0;
	complex	w = {c->r, c->i};
	double	buff;

	while (w.r * w.r + w.i * w.i <= 2 && i <= max)
	{
		step(&w, c);
		it++;
	}
	return i;
}