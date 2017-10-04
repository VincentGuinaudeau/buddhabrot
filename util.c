#include <stdlib.h>

void	*my_free(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
	return (NULL);
}