#ifndef PARSER_H_
#define PARSER_H_

#include "definitions.h"

err long_parse_number(char *str, long *number, int min);
err parse_number(char *str, int *number, int min);
err parse_double(char *str, double *number);
err parse_complex(char *str, complex *c);
err parse_enum(char *str, int *result, char **values, int array_length);

#endif /* PARSER_H_ */