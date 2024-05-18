#include <stdint.h>
#include "header/filesystem/user-shell.h"
#include "header/filesystem/command.h"
#include "header/stdlib/string.h"
#include "header/driver/disk.h"
#include "header/stdlib/stdtype.h"

uint32_t current_directory = ROOT_CLUSTER_NUMBER;
struct FAT32DirectoryTable dir_table;

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

void printn(char *buf, uint8_t color, int n){
    interrupt(5, (uint32_t) buf, n, color);
}

void parser(char *buf, char *firstbuf, char *secondbuf){
    const uint8_t *buf2 = (const uint8_t*) buf;
    int index = 0;
    bool found = false;
    int len = strlen(buf);
    if (strlen(buf) == 2) {
        index = 2;
        splitfirst(buf2, firstbuf, index+1);
        found=true;
    }
    else {
        for (int i = 0; i < len; i++) {
            if (buf2[i] == ' ' && !found){ 
                index = i;
                memcpy(buf, firstbuf, index+1);
                splitsecond(buf2, secondbuf, index+1,len);
                found = true;
            }
        }

    }

    if (!found){
        // invalid
        print("Invalid command!\n", BIOS_RED);
    }
}

void updateDirectoryTable(uint32_t cluster_number) {
    interrupt(6, (uint32_t) &dir_table, cluster_number, 0x0);
}

int findEntryName(char* name) {
    int result = -1;

    int i = 1;
    bool found = false;
    while (i < 64 && !found) {
        if (memcmp(dir_table.table[i].name, name, 8) == 0 && 
            dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            result = i;
            found = true;
        }
        else {
            i++;
        }
    }

    return result;
}

void splashScreen() {
    print("\n\n\n\n\n", BIOS_PINK);
    print("                                 Welcome to\n", BIOS_PINK);
    print("            ____    U  ___ u   ____   __   __  U  _    u ____     \n", BIOS_PINK);
    print("         U / ___|u   \\/ _ \\/U | __ )u \\ \\ / /  \\/ _ \\/ /__   | u \n", BIOS_PINK);
    print("         \\| |  _ /   | | | | \\|  _ \\/  \\ V /   | | | |<\\___ \\/  \n", BIOS_PINK);
    print("          | |_| |.-,_| |_| |  | |_) | U_| |_u.-,_| |_| | u___) |   \n", BIOS_PINK);
    print("          \\____|  \\_)-\\___/   |____/    |_| \\_)-\\___/  |____/>> \n", BIOS_PINK);
    print("           _)(|_       \\      _|| \\_.-, //|(_       \\     )(  (__)\n", BIOS_PINK);
    print("          (__)__)     (__)    (__) (__) \\_) (__)     (__)   (__)   \n", BIOS_PINK);
    print("           \n", BIOS_PINK);
    print("                          Press enter to continue!\n\n", BIOS_PINK);
}

int main(void) {
    char arg[26];
    char args_val[2048];
    char cmd[6] = {'\0','\0','\0','\0','\0','\0'};
    char args[15] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char path_str[26];
    
    splashScreen();
    interrupt (4, (uint32_t) args_val, 2048, 0x0);
    interrupt(7, 0, 0, 0);

    updateDirectoryTable(current_directory);

    while (true) {
        clear(path_str, 26);
        clear(args, 15);
        clear(arg, 26);
        clear(cmd, 6);

        print("GobyOS-IF2230", BIOS_LIGHT_GREEN);
        print(":", BIOS_GREY);
        print ("/", BIOS_LIGHT_BLUE);
        if (memcmp(dir_table.table->name, "root",4)!=0 || strcmp(dir_table.table->name, "")!=0 ){
            print(dir_table.table->name, BIOS_LIGHT_BLUE);
            print(" ", BIOS_GREY);
        }

        interrupt (4, (uint32_t) arg, 26, 0x0);
        const uint8_t *buf2 = (const uint8_t*) arg;
        int index = 0;
        bool found = false;
        int len = strlen(arg);

        if (strlen(arg) == 2) {
            index = 2;
            splitfirst(buf2, cmd, index+1);
            found=true;
        }
        else {
            for (int i = 0; i < len; i++) {
                if (buf2[i] == ' ' && !found){ 
                    index = i;
                    splitfirst(buf2, cmd, index);
                    found = true;
                }
            }

        }

        if (!found){
            // invalid
            print("Invalid command!\n", BIOS_RED);
        }


        if (strcmp(cmd, "cd")==0){
            splitsecond(buf2, args, index+1,len);
            cd(args);
        } 
        else if (strcmp(cmd, "mkdir")==0){
            splitsecond(buf2, args, index+1,len);
            mkdir(args);
        }
        else if (strcmp(cmd, "ls")==0){
            ls();
        }
        else if (strcmp(cmd, "cp")==0){
            size_t len2 = len-index+1;
            memcpy2(args,buf2,index+1,len2);
            cp(args);
        }
        else if (strcmp(cmd, "cat")==0){
            splitsecond(buf2, args, index+1,len);
            cat(args);
        }
        else if (strcmp(cmd, "mv")==0){
            splitsecond(buf2, args, index+1,len);
            mv(args);
        }
        else if (strcmp(cmd, "rm")==0){
            splitsecond(buf2, args, index+1,len);
            rm(args);
        }
        else if (strcmp(cmd, "find")==0){
            int curr2 = current_directory;
            splitsecond(buf2, args, index+1,len);
            find(args);
            updateDirectoryTable(curr2);
        } else{
            print("Invalid command!\n", BIOS_RED);
        }
    }
     return 0;
}
    

        