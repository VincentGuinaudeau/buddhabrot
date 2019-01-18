#include <stdio.h>
#define TYPE double
#define TYPE_S "double"

int main()
{
	printf("size of "TYPE_S" : %lu\n", sizeof(TYPE));

	TYPE step = 1;
	TYPE prev = 0;
	TYPE test = 1;
	int i = 0;
	while (prev != test)
	{
		i++;
		step /= 2;
		prev = test;
		test += step;
	}
	printf("test on border with type "TYPE_S" : %d step\n", i);

	step = 1;
	prev = 0;
	test = 1;
	i = 0;
	while (prev != test)
	{
		i++;
		step /= 2;
		prev = test;
		test = test + (i % 2 ? step : -step);
	}
	printf("test alterning with type "TYPE_S" : %d step\n", i);

	step = 1;
	prev = 0;
	test = 1;
	i = 0;
	while (prev != test)
	{
		i++;
		step /= 2;
		prev = test;
		test -= step;
	}
	printf("test on middle with type "TYPE_S" : %d step\n", i);
}