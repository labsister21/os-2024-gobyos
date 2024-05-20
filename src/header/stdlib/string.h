#include "stdtype.h"
#include "stdmem.h"
#include <stddef.h>

#ifndef _STRING_H_
#define _STRING_H_

size_t strlen(char *string);

uint8_t strcmp(char *s1, char *s2);

void clear(void *pointer, size_t n);

void strcpy(char *str_dest, char *str_src);

void splitfirst(const uint8_t* restrict src, void* first, int offset) ;

void splitsecond(const uint8_t* restrict src, void* second, int offset, int len) ;
void itoa(int32_t value, char *result);


#endif