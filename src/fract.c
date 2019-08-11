#include "fract.h"

// double cos(double x);
// double sin(double x);
// double atan2(double y, double x);
// double pow(double x, double y);
// double sqrt(double x);

inline double distance_squared(double x, double y)
{
	return x * x + y * y;
}

void complex_multiply(complex *c1, complex *c2, complex *result)
{
	double buffer = c1->r * c2->r - c1->i * c2->i;
	result->i = c1->r * c2->i - c1->i * c2->r;
	result->r = buffer;
}

void complex_pow_and_add(complex *c, double power, complex *add, complex *result)
{
	double module = pow(sqrt(c->r * c->r + c->i * c->i), power);
	double angle = atan2(c->i, c->r) * power;
	result->r = cos(angle) * module + add->r;
	result->i = sin(angle) * module + add->i;
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
		// calc_step(&walker, point);
		++i;
	}
	return i;
}

void calc_step(fract_params *f_params, trace *trace, complex *c, complex *result)
{
	complex add;
	switch (f_params->type)
	{
		case mandelbrot:
			complex_multiply(&f_params->factor, trace->points, &add);
			complex_pow_and_add(c, f_params->power, &add, result);
		break;

		case julia:
			complex_multiply(&f_params->factor, &f_params->point, &add);
			complex_pow_and_add(c, f_params->power, &add, result);
		break;
	}
}

void init_trace(__attribute__((unused)) fract_params *f_param, trace *trace, double r, double i)
{
	trace->length = 0;
	trace->points[0].r = r;
	trace->points[0].i = i;
}

void compute_trace(fract_params *f_params, trace *trace, int max)
{
	while (
		distance_squared(
			trace->points[trace->length].r,
			trace->points[trace->length].i
		) <= 4 &&
		trace->length <= max
	)
	{
		calc_step(f_params, trace, trace->points + trace->length, trace->points + trace->length + 1);
		trace->length++;
	}
}