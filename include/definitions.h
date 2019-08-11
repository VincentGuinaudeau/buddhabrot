#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef int err;
enum {KO, OK};

typedef int bool;
enum {false, true};

typedef struct s_complex
{
	double	r;
	double	i;
}	complex;

#endif /* DEFINITIONS_H_ */