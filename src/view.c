
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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
	unsigned char buffer[WRITE_BUFFER_SIZE];
	const int max_pix = WRITE_BUFFER_SIZE / PIXEL_SIZE;
	int fd = open(path, O_CREAT | O_RDWR);
	int size = view->x * view->y - 1;
	int progress = 0;
	int buffer_index;
	int buff;
	int i;

	if (fd == -1)
		return KO;
	sprintf((char*)buffer, "P5 %d %d %d\n", view->x, view->y, PGM_MAX_VALUE);
	write(fd, buffer, strlen((char*)buffer));
	for (i = 0; i <= size; i++)
	{
		buff = view->data[i] * PGM_MAX_VALUE / view->max_value;
		buffer_index = (i % max_pix) * PIXEL_SIZE;
		#if PIXEL_SIZE >= 1
			buffer[buffer_index] = buff >> 8;
			buffer[buffer_index + 1] = buff;
		#else
			buffer[buffer_index] = buff;
		#endif
		if ((i + 1) % max_pix == 0)
		{
			write(fd, buffer, max_pix * PIXEL_SIZE);
		}
		buff = (long)i * 10000 / size;
		if (progress < buff)
		{
			progress = buff;
			printf("\r%d.%d%d%%", progress / 100, progress / 10 % 10, progress % 10);
			fflush(stdout);
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
	}
}

void prepare_point_for_view(view *view, int *buffer, complex *point)
{
	complex walker = {point->r, point->i};
	int x;
	int y;
	int i = 0;

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
			buffer[i] = y * view->x + x;
			++i;
		}
		calc_step(&walker, point);
	}
	buffer[i] = -1;
}

void add_computed_point_to_view(view *view, int *buffer)
{
	int i = 0;

	while (buffer[i] != -1)
	{
		++view->data[buffer[i]];
		if (view->max_value < view->data[buffer[i]])
			view->max_value = view->data[buffer[i]];
		++i;
	}
}

void add_point_to_view(view *view, complex *point)
{
	complex walker = {point->r, point->i};
	int x;
	int y;
	int index;

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
			index = y * view->x + x;
			++view->data[index];
			if (view->max_value < view->data[index])
				view->max_value = view->data[index];
		}
		calc_step(&walker, point);
	}
}