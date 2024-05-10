#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/driver/disk.h"
#include "header/stdlib/stdtype.h"

void interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    // so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print(char *buf, uint8_t color){
    interrupt(5, (uint32_t) buf, strlen(buf), color);
}


int main(void) {
    // char buf[255];
    // buf[254] = '\0';
    char args_val[2048];
    // int args_info[128][2];
    // char path_str[2048];

   print("GobyOS-IF2230", BIOS_LIGHT_GREEN);
    interrupt (4, (uint32_t) args_val, 2048, 0x0);
    interrupt(7, 0, 0, 0);
    while (true) {
        print("GobyOS-IF2230", BIOS_LIGHT_GREEN);
        print(":", BIOS_GREY);
        interrupt (4, (uint32_t) args_val, 2048, 0x0);
        interrupt(7, 0, 0, 0);
        // char *arg = buf;`
        // uint8_t len;
        // if (strcmp(arg, "cd")==1){

        // } 
        // else if (!strcmp(arg, "mkdir")==1){

        // }
        // else if (strcmp(arg, "ls")==1){

        // }
        // else if (strcmp(arg, "cp")==1){

        // }
        // else if (strcmp(arg, "cat")==1){

        // }
        // else if (strcmp(arg, "mv")==1){

        // }
        // else if (strcmp(arg, "rm")==1){

        // }
        // else if (strcmp(arg, "cp")==1){

        // }
        // else if (strcmp(arg, "find")==1){

        // }
    return 0;
    }
}
    

        