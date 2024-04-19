#ifndef _STDMEM_H
#define _STDMEM_H

#include "stdtype.h"

void* memcpy(void* restrict dest, const void* restrict src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

#endif