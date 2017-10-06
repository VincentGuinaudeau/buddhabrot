#ifndef MAIN_H_
#define MAIN_H_

typedef struct s_option
{
	int		width;
	int		height;
	int		sample_size;
	int		min;
	int		max;
	double	scale;
	double	x_offset;
	double	y_offset;
} t_option;

typedef int err;
enum {KO, OK};

typedef int bool;
enum {false, true};

#endif /* MAIN_H_ */