
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "view.h"

double sqrt(double x);
double log(double x);
double pow(double x, double y);

double calc_squared_view_radius(double scale, int x, int y)
{
	double small_scale;

	if (x > y)
		small_scale = scale * (double)y / (double)x;	
	else
		small_scale = scale * (double)x / (double)y;

	return (scale * scale + small_scale * small_scale);
}

void find_gamma_for_optimal_median(view *view)
{
	long i;
	int *counters = calloc(sizeof(int), view->max_value + 1);

	long data_size = view->x * view->y;
	for (i = 0; i < data_size; i++)
	{
		counters[view->data[i]]++;
	}

	long median_point = (data_size - counters[0]) / 2;
	long accumulator = 0;
	for (i = 1; i <= view->max_value; i++)
	{
		accumulator += counters[i];
		if (accumulator >= median_point)
			break;
	}

	long median = i;

	view->gamma = 1.0 / ((double)log((1.0 / (double)view->max_value) * median) / log(0.5));
	view->gamma = sqrt(view->gamma);
}

void set_view_position(view *view, double scale, complex *offset)
{
	view->scale = scale;
	view->offset = *offset;
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

view *clone_view(view *source)
{
	view *new_view = create_view(source->x, source->y);
	if (new_view == NULL)
		return NULL;

	new_view->scale          = source->scale;
	new_view->squared_radius = source->squared_radius;
	new_view->offset         = source->offset;
	new_view->step           = source->step;
	new_view->render_type    = source->render_type;
	new_view->gamma          = source->gamma;
	return new_view;
}

err merge_view(view *destination, view *source)
{
	if (destination->x * destination->y != source->x * source->y)
		return KO;

	long length = destination->x * destination->y;
	for (int i = 0; i < length; i++)
	{
		destination->data[i] += source->data[i];
		if (destination->max_value < destination->data[i])
		{
			destination->max_value = destination->data[i];
		}
	}
	return OK;
}

err write_view_to_disk(view *view, char *path)
{
	unsigned char buffer[WRITE_BUFFER_SIZE];
	const int max_pix = WRITE_BUFFER_SIZE / PIXEL_SIZE;
	int fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	int size = view->x * view->y - 1;
	int progress = 0;
	int buffer_index;
	int buff;
	long i;

	if (view->gamma == -1)
	{
		find_gamma_for_optimal_median(view);
		printf("automatic gamma : %lf\n", view->gamma);
	}

	if (fd == -1)
		return KO;
	sprintf((char*)buffer, "P5 %d %d %d\n", view->x, view->y, PGM_MAX_VALUE);
	if (write(fd, buffer, strlen((char*)buffer)) == -1)
	{
		printf("write failed.\n");
		return KO;
	}
	for (i = 0; i <= size; i++)
	{
		if (view->gamma)
		{
			buff = PGM_MAX_VALUE * pow((double)view->data[i] / (double)view->max_value, view->gamma);
		}
		else
		{
			buff = view->data[i] * PGM_MAX_VALUE / view->max_value;
		}
		buffer_index = (i % max_pix) * PIXEL_SIZE;
		#if PIXEL_SIZE >= 1
			buffer[buffer_index] = buff >> 8;
			buffer[buffer_index + 1] = buff;
		#else
			buffer[buffer_index] = buff;
		#endif
		if ((i + 1) % max_pix == 0)
		{
			if (write(fd, buffer, max_pix * PIXEL_SIZE) == -1)
			{
				printf("write failed.\n");
				return KO;
			}
		}
		buff = (long)i * 10000 / size;
		if (progress < buff)
		{
			progress = buff;
			printf("\r%d.%d%d%%", progress / 100, progress / 10 % 10, progress % 10);
			fflush(stdout);
		}
	}
	if ((i + 1) % max_pix != 0)
	{
		if (write(fd, buffer, i % max_pix * PIXEL_SIZE) == -1)
		{
			printf("write failed.\n");
			return KO;
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

void prepare_point_for_view(view *view, int *buffer, complex *point, int max)
{
	complex walker = {point->r, point->i};
	int x;
	int y;
	int i = 0;

	while (distance_squared(walker.r - view->offset.r,
	                        walker.i - view->offset.i)
	       <= view->squared_radius)
	{
		x = (walker.r - view->offset.r) / view->step + view->x / 2;
		y = (walker.i - view->offset.i) / view->step + view->y / 2;
		if (x >= 0 && x < view->x && y >= 0 && y < view->y)
		{
			buffer[i] = y * view->x + x;
			++i;
			break;
			if (i > max)
				return;
		}
		// calc_step(&walker, point);
	}
	buffer[i] = -1;
}

void add_computed_point_to_view(view *view, int *buffer)
{
	int i = 0;

	while (buffer[i] != -1)
	{
		// view->data[buffer[i]] = 255;
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

	while (distance_squared(walker.r - view->offset.r,
	                        walker.i - view->offset.i)
	    <= view->squared_radius)
	{
		x = (walker.r - view->offset.r) / view->step + view->x / 2;
		y = (walker.i - view->offset.i) / view->step + view->y / 2;
		if (x >= 0 && x < view->x && y >= 0 && y < view->y)
		{
			index = y * view->x + x;
			++view->data[index];
			if (view->max_value < view->data[index])
				view->max_value = view->data[index];
		}
		// calc_step(&walker, point);
	}
}

inline void add_to_view(view *view, int x, int y, int value)
{
	int index = y * view->x + x;
	view->data[index] += value;
	if (view->max_value < view->data[index])
		view->max_value = view->data[index];
}

inline bool insert_complex_in_view(view *view, complex *c, int value)
{
	int x = (c->r - view->offset.r) / view->step + view->x / 2;
	int y = (c->i - view->offset.i) / view->step + view->y / 2;
	if (x >= 0 && x < view->x && y >= 0 && y < view->y)
	{
		add_to_view(view, x, y, value);
		return true;
	}
	else
	{
		return false;
	}
}

void add_trace_to_view(view *view, fract_params *f_params, trace *trace)
{
	bool is_in_view = false;
	int i;

	switch (view->render_type)
	{
		case binary:
			insert_complex_in_view(view, trace->points, 1);
		break;

		case layered:
			insert_complex_in_view(view, trace->points, trace->length + 1);
		break;

		case buddhabrot:
			for (i = 0; i <= trace->length; i++)
			{
				is_in_view = insert_complex_in_view(view, trace->points + i, 1);
			}
			complex walker = trace->points[trace->length + 1];
			// continuing the trace, to include points outside the circle
			walker = trace->points[trace->length + 1];
			while (is_in_view)
			{
				calc_step(f_params, trace, &walker, &walker);
				is_in_view = insert_complex_in_view(view, &walker, 1);
			}
			for (i = 0; i < 2; i++)
			{
				calc_step(f_params, trace, &walker, &walker);
				insert_complex_in_view(view, &walker, 1);
			}
		break;
	}
}