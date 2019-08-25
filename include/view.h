#ifndef VIEW_H_
#define VIEW_H_

#include "definitions.h"

typedef enum e_render_type {
	binary,
	layered,
	buddhabrot
} render_type;

typedef struct s_view
{
	int			x;
	int			y;
	double		scale;
	complex		offset;
	double		squared_radius;
	double		step;
	double		gamma;
	render_type render_type;
	int			max_value;
	int			data[];
}	view;

#include "fract.h"

// use 255   to store optimaly each pixel in 1 octet
// use 65535 to store optimaly each pixel in 2 octet
#define PGM_MAX_VALUE		65535

#define WRITE_BUFFER_SIZE	4096

#if PGM_MAX_VALUE > 255
	#define PIXEL_SIZE 2
#else
	#define PIXEL_SIZE 1
#endif

void set_view_position(view *view, double scale, complex *offset);
view *create_view(int x, int y);
view *clone_view(view *source);
err merge_view(view *destination, view *source);
err write_view_to_disk(view *view, char *path);
void clean_view(view *view);
void prepare_point_for_view(view *view, int *buffer, complex *point, int max);
void add_computed_point_to_view(view *view, int *buffer);
void add_point_to_view(view *view, complex *point);
int add_trace_to_view(view *view, fract_params *f_params, trace *trace);

#endif /* VIEW_H_ */