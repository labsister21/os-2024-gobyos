#include "header/filesystem/command.h"
#include "header/filesystem/user-shell.h"
#include "header/stdlib/stdtype.h"

void cat(char *fileName){
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    updateDirectoryTable(search_directory_number);


    int index = 0;
    int buf_len = strlen(fileName);
    bool valid = false;
    for (int i = 0; i < buf_len; i++) {
        if (fileName[i] == '.'){ 
            index = i;
            valid = true;
            splitname(fileName, srcName, srcExt, index+1);
            break;
        }
    }
 
    int entry_index = findEntryName(srcName);
    if (entry_index != -1) {
        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
            search_directory_number =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
            updateDirectoryTable(search_directory_number);
        } else if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
            search_directory_number =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
            updateDirectoryTable(search_directory_number);
        }
    }

    if(valid){
        int32_t retcode;

        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest request = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = ROOT_CLUSTER_NUMBER,
            .buffer_size = 4 * CLUSTER_SIZE,
        };
        
        memcpy(&(request.name), srcName, 8);
        memcpy(&(request.ext), srcExt, 3);
        interrupt(0, (uint32_t) &request, (uint32_t) &retcode, 0x0);

        if (retcode != 0 ) {
            switch (retcode) {
                case 1:
                    print("cat : Is a directory\n", BIOS_RED);
                    break;
                case 2:
                    print("cat : Buffer size is not enough\n", BIOS_RED);
                    break;
                case 3:
                    print("cat : No such file or directory\n", BIOS_RED);;
                    break;
            }
        } else {
                print((char *) &cl, BIOS_BROWN);
        }
    } else{
        print("cat : No such file or directory\n", BIOS_RED);
    }
    
}

void splitname(char* buf, char* first, char* second, int offset) {
    int len = strlen(buf);

    // buat kata pertama
    for (int i = 0; i < len; i++) {
        if(buf[i]=='\0' || buf[i]=='.'){
            break;
        }
        first[i] = buf[i];
    }

    // buat kata kedua
    for (int i = 0; i < len - offset; i++) {
        if(buf[i]=='\0'){
            break;
        }
        second[i] = buf[i + offset];
    }

}