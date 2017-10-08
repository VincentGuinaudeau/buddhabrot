
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "view.h"

double calc_squared_view_radius(double scale, int x, int y)
{
	double ratio;

	if (x > y)
		ratio = (double)x / (double)y;
	else
		ratio = (double)y / (double)x;

	ratio *= scale;
	return (scale * scale + ratio * ratio);
}

void set_view_position(view *view, double scale, double x, double y)
{
	view->scale = scale;
	view->offset_x = x;
	view->offset_y = y;
	view->squared_radius = calc_squared_view_radius(scale, view->x, view->y);
	view->step = (view->scale * 2) / (view->x < view->y ? view->x : view->y);
}

view *create_view(int x, int y)
{
	view *new_view;

	new_view = calloc(1, sizeof(view) + sizeof(int) * x * y);
	if (new_view == NULL)
		return NULL;

	new_view->x = x;
	new_view->y = y;
	new_view->scale = 2;
	new_view->squared_radius = calc_squared_view_radius(2, x, y);
	new_view->step = (new_view->scale * 2) /
	                 (new_view->x < new_view->y ? new_view->x : new_view->y);
	new_view->max_value = 1;
	return new_view;
}

err write_view_to_disk(view *view, char *path)
{
	int fd = open(path, O_CREAT | O_RDWR);
	int size = view->x * view->y - 1;
	int progress = 0;
	int buff;
	int i;

	if (fd == -1)
		return KO;
	dprintf(fd, "P5 %d %d %d\n", view->x, view->y, PGM_MAX_VALUE);
	for (i = 0; i <= size; i++)
	{
		buff = view->data[i] * PGM_MAX_VALUE / view->max_value;
		#if PGM_MAX_VALUE > 255
			dprintf(fd, "%c%c", buff >> 8, buff);
		#else
			dprintf(fd, "%c", buff);
		#endif
		buff = (long)i * 10000 / size;
		if (progress < buff)
		{
			progress = buff;
			printf("\r%d.%d%d%%", progress / 100, progress / 10 % 10, progress % 10);
		}
	}
	printf("\n");
	close(fd);
	return OK;
}

void clean_view(view *view)
{
	long i = view->x * view->y;

	while (i)
	{
		view->data[--i] = 0;
		i--;
	}
}

void add_point_to_view(view *view, complex *point)
{
	complex walker = {point->r, point->i};
	int x;
	int y;

	while (distance_squared(walker.r - view->offset_x,
		                    walker.i - view->offset_y)
		<= view->squared_radius
	    && distance_squared(walker.r,
	    	                walker.i)
	    <= 4)
	{
		x = (walker.r - view->offset_x) / view->step + view->x / 2;
		y = (walker.i - view->offset_y) / view->step + view->y / 2;
		if (x >= 0 && x < view->x && y >= 0 && y < view->y)
		{
			view->data[y * view->x + x]++;
			if (view->max_value < view->data[y * view->x + x])
				view->max_value = view->data[y * view->x + x];
		}
		calc_step(&walker, point);
	}
}