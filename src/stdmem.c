#include "header/stdlib/stdtype.h"
#include "header/stdlib/stdmem.h"
#include "header/stdlib/string.h"

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    uint8_t *buf1 = (uint8_t*) dest;
    const uint8_t *buf2 = (const uint8_t*) src;
    for (size_t i = 0; i < n; i++){
        buf1[i] = buf2[i];
    }       
    return buf1;
}

int memcmp (const void *s1, const void *s2, size_t n) {
    const uint8_t *buf1 = (const uint8_t*) s1;
    const uint8_t *buf2 = (const uint8_t*) s2;
    for (size_t i = 0; i < n; i++) {
        if (buf1[i] < buf2[i])
            return -1;
        else if (buf1[i] > buf2[i])
            return 1;
    }

    return 0;
}
