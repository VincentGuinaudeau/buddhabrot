#ifndef FRACT_H_
#define FRACT_H_

#include "definitions.h"

typedef struct s_trace
{
	int		length;
	complex points[];
}	trace;

typedef enum e_fract_type
{
	mandelbrot,
	julia,
}	fract_type;

typedef struct s_fract_params
{
	fract_type	type;
	complex		point;
	double		power;
	complex		factor;
}	fract_params;

#include <math.h>
#include "main.h"

void calc_step(fract_params *f_params, trace *trace, complex *c, complex *result);
double distance_squared(double x, double y);
int number_of_step_to_escape(complex *c, int max);
void init_trace(fract_params *f_param, trace *trace, double r, double i);
void compute_trace(fract_params *f_params, trace *trace, int max);

#endif /* FRACT_H_ */