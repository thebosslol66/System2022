#ifndef H_UTILS
#define H_UTILS

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

unsigned int randInt(unsigned int max);
int have_space(char * str);
void flush(FILE * fp);
size_t count_file_lines(FILE * fp);

#endif //H_UTILS