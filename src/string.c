#include "header/stdlib/string.h"

void clear(void *pointer, size_t n) {
    uint8_t *ptr       = (uint8_t*) pointer;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = 0x00;
    }
}

size_t strlen(char *string) {
    size_t len = 0;
    while (string[len] != '\0'){
        len++;
    }
    return len;
}

uint8_t strcmp(char *s1, char *s2) {
    size_t i = 0;
    if (strlen(s1) == strlen(s2)) {
        while (s1[i] != '\0') {
            if (s1[i] != s2[i])
                return 1;
            i++;
        }
        return 0;
    }
    else{
        return 1;
    }
}

void strcpy(char *str_dest, char *str_src){
    int i = 0;
    while(str_src[i]!='\0'){
        str_dest[i] = str_src[i];
        i++;
    }
    str_dest[i] = '\0';
    return;
}

void splitfirst(const uint8_t* restrict src, void* first, int offset) {
    const uint8_t *buf2 = (const uint8_t*) src;
    memcpy(first,buf2,offset);  
}

void splitsecond(const uint8_t* restrict src, void* second, int offset, int len) {
    const uint8_t *buf2 = (const uint8_t*) src;
    size_t len2 = len-offset;
    memcpy2(second,buf2,offset,len2);
}


void itoa(int32_t value, char *result)
{
    char *ptr = result, *ptr1 = result, tmp_char;
    int32_t tmp_value;

    do
    {
        tmp_value = value;
        value /= 10;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * 10)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0)
        *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}