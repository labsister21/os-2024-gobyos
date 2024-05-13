#include "header/filesystem/command.h"
#include "header/filesystem/user-shell.h"
#include "header/stdlib/stdtype.h"

void cat(char *fileName){
    struct ClusterBuffer cl           = {0};
    struct FAT32DriverRequest request = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0",
        .ext = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size = 4 * CLUSTER_SIZE,
    };

    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    int index = 0;
    int buf_len = strlen(fileName);
    for (int i = 0; i <= buf_len; i++) {
        if (fileName[i] == '.'){
            index = i;
            break;
        }
        index = i;
    }

    for (uint8_t j = 0; j < index; j++) {
        srcName[j] = fileName[j];
    }
    srcName[index] = '\0';
    memcpy((request.name), srcName, 8);
    if(buf_len!=index){
        // buat extension
        for (uint8_t i = 1; i < (strlen(fileName) - index); i++) {
            srcExt[i] = fileName[i + index];
        }
        srcExt[strlen(fileName) - index] = '\0';
        memcpy((request.ext), srcExt, 3);
    }


    int32_t retcode;

    interrupt(0, (uint32_t) &request, (uint32_t) &retcode, 0x0);
    if (retcode != 0) {
        print("cat:", BIOS_RED);
        switch (retcode) {
            case 1:
                print(": Is a directory\n", BIOS_RED);
                break;
            case 2:
                print(": Buffer size is not enough\n", BIOS_RED);
                break;
            case 3:
                print(": No such file or directory\n", BIOS_RED);
                break;
        }
    } else {
        print(request.buf, BIOS_BROWN);
    }
}

