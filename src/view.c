
#include <stdlib.h>
#include "view.h"

inline double calc_view_radius(double scale, int x, int x)
{
	double ratio = (double)x / (double)y;

}

view *create_view(int x, int y)
{
	view *new_view;

	if ((new_view = calloc(1, sizeof(view) + sizeof(int) * x * y)) == NULL)
	{
		return NULL;
	}
	new_view->x = x;
	new_view->y = y;
	return new_view;
}

err write_view_to_disk(view *view, char *path)
{

}

void add_point_to_view(view *view, complex *point)
{
	complex walker;
}