#include <stdint.h>
#include "header/filesystem/user-shell.h"
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
    char arg[26];

    while (true) {
        print("GobyOS-IF2230", BIOS_LIGHT_GREEN);
        print(":", BIOS_GREY);
        interrupt (4, (uint32_t) arg, 26, 0x0);
        if (strcmp(arg, "cd")==0){

        } 
        else if (strcmp(arg, "mkdir")==0){

        }
        else if (strcmp(arg, "ls")==0){

        }
        else if (strcmp(arg, "cp")==0){

        }
        else if (strcmp(arg, "cat")==0){

        }
        else if (strcmp(arg, "mv")==0){

        }
        else if (strcmp(arg, "rm")==0){

        }
        else if (strcmp(arg, "cp")==0){

        }
        else if (strcmp(arg, "find")==0){

        }
        else{
            print("Invalid command !\n", BIOS_RED);
        }
    }
     return 0;
}
    

        