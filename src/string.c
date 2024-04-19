#include "header/stdlib/string.h"

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

void* memset(void *s, uint8_t c, size_t n) {
    uint8_t *buf = (uint8_t*) s;
    for (size_t i = 0; i < n; i++)
        buf[i] = c;
    return s;
}