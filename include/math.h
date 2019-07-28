#ifndef MATH_H_
#define MATH_H_

typedef struct s_complex
{
	double	r;
	double	i;
} complex;

typedef struct s_trace
{
	int		length;
	complex points[];
} trace;

#include "main.h"

double distance_squared(double x, double y);
void calc_step(complex *walker, complex *start_point);
int number_of_step_to_escape(complex *c, int max);
void compute_trace(trace *trace, int max);

#endif /* MATH_H_ */