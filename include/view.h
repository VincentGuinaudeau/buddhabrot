#ifndef VIEW_H_
#define VIEW_H_

typedef struct s_view
{
	int		x;
	int		y;
	double	scale;
	double	offset_x;
	double	offset_y;
	double	squared_radius;
	double	step;
	int		max_value;
	int		data[];
}	view;

#include "main.h"
#include "math.h"

#define WRITE_BUFFER_SIZE	4096
#define PGM_MAX_VALUE		255

#if PGM_MAX_VALUE > 255
	#define PIXEL_SIZE 2
#else
	#define PIXEL_SIZE 1
#endif

void set_view_position(view *view, double scale, double x, double y);
view *create_view(int x, int y);
err write_view_to_disk(view *view, char *path);
void clean_view(view *view);
void add_point_to_view(view *view, complex *point);

#endif /* VIEW_H_ */