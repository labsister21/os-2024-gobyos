#include <stdint.h>
#include "header/filesystem/user-shell.h"
#include "header/filesystem/command.h"
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

void parser(char *buf, char *firstbuf, char *secondbuf){
    int index = 0;
    bool found = false;
    if (strlen(buf) == 2) {
        index = 2;
        split(buf, firstbuf, secondbuf, index+1);
        found=true;
    }
    else {
        int buf_len = strlen(buf);
        for (int i = 0; i < buf_len; i++) {
            if (buf[i] == ' '){ 
                index = i;
                split(buf, firstbuf, secondbuf, index+1);
                found = true;
            }
        }

    }

    if (!found){
        // invalid
        print("Invalid command!\n", BIOS_RED);
    }
}


int main(void) {
    char arg[26];
    char cmd[12];
    char args[12];

    while (true) {

        // clear(args, 26);
        // clear(arg, 26);
        // clear(cmd, 4);

        print("GobyOS-IF2230", BIOS_LIGHT_GREEN);
        print(":", BIOS_GREY);
        interrupt (4, (uint32_t) arg, 26, 0x0);
        parser(arg, cmd, args);

        if (strcmp(cmd, "cd")==0){

        } 
        else if (strcmp(cmd, "mkdir")==0){

        }
        else if (strcmp(cmd, "ls")==0){

        }
        else if (strcmp(cmd, "cp")==0){

        }
        else if (strcmp(cmd, "cat")==0){
            // parser(arg, cmd, args);
            cat(args);
        }
        else if (strcmp(cmd, "mv")==0){

        }
        else if (strcmp(cmd, "rm")==0){

        }
        else if (strcmp(cmd, "cp")==0){

        }
        else if (strcmp(cmd, "find")==0){

        } 
    }
     return 0;
}
    

        