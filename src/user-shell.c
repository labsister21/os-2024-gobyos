#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    // so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void put(char *buf, uint8_t color){
    interrupt(5, (uint32_t) buf, strlen(buf), color);
}

int main(void) {

    while (true) {
        put("GobyoS-IF2230", BIOS_LIGHT_GREEN);
        put(":", BIOS_GREY);
    }
    return 0;
    }

        