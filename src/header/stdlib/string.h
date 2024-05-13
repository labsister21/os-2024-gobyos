#include "stdtype.h"
#include <stddef.h>

#ifndef _STRING_H_
#define _STRING_H_

size_t strlen(char *string);

uint8_t strcmp(char *s1, char *s2);

void clear(void *pointer, size_t n);

void strcpy(char *str_dest, char *str_src);

void split(char* buf, char* first_section, char* second_section, int offset) ;

#endif