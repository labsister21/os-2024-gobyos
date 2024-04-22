#include "stdtype.h"
#include <stddef.h>

#ifndef _STRING_H_
#define _STRING_H_

size_t strlen(char *string);

uint8_t strcmp(char *s1, char *s2);

void clear(void *pointer, size_t n);

#endif