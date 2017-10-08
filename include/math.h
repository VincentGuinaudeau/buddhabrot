#ifndef MATH_H_
#define MATH_H_

#include "main.h"

typedef struct s_complex
{
	double	r;
	double	i;
} complex;

double distance_squared(double x, double y);
void calc_step(complex *walker, complex *start_point);
int number_of_step_to_escape(complex *c, int max);

#endif /* MATH_H_ */