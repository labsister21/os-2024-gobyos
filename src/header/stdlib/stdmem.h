#ifndef _STDMEM_H
#define _STDMEM_H

#include <stddef.h>

void* memcpy(void* restrict dest, const void* restrict src, size_t n);

void* memcpy2(void* restrict dest, const void* restrict src, size_t n, size_t leb);

int memcmp(const void *s1, const void *s2, size_t n);

void* memset(void *s, int c, size_t n);

void *memmove(void *dest, const void *src, size_t n);

#endif