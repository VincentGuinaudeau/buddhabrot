#ifndef MAIN_H_
#define MAIN_H_

typedef struct s_option
{
	int	width;
	int	height;
	int point_number;
	int min;
	int max;
} t_option;

typedef int err;

#define OK 0
#define KO !OK

#endif /* MAIN_H_ */