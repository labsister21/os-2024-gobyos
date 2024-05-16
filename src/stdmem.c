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

void* memcpy2(void* restrict dest, const void* restrict src, size_t n, size_t len) {
    uint8_t *buf1 = (uint8_t*) dest;
    const uint8_t *buf2 = (const uint8_t*) src;
    for (size_t i = 0; i < len; i++){
        buf1[i] = buf2[i+n];
    }
    buf1[len] = '\0';       
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

void* memset(void *s, int c, size_t n) {
    uint8_t *buf = (uint8_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = (uint8_t) c;
    return s;
}


void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *dstbuf       = (uint8_t*) dest;
    const uint8_t *srcbuf = (const uint8_t*) src;
    if (dstbuf < srcbuf) {
        for (size_t i = 0; i < n; i++)
            dstbuf[i]   = srcbuf[i];
    } else {
        for (size_t i = n; i != 0; i--)
            dstbuf[i-1] = srcbuf[i-1];
    }

    return dest;
}