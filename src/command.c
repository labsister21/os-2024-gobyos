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

            .parent_cluster_number = search_directory_number,
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

void cp(char* value) {
    char source[12];
    char dest[12];

    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    char dstName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char destExt[3] = {'\0','\0','\0'};

    int index = 0;
    int buf_len = strlen(value);
    const uint8_t *buf2 = (const uint8_t*) value;
    for (int i = 0; i < buf_len; i++) {
        if (value[i] == ' '){ 
            index = i;
            splitfirst(buf2, source, index+1);
            splitsecond(buf2, dest, index+1,buf_len);
            break;
        }
    }

    if (strlen(value)==0 || dest[0]=='\0') {
        print("cp: missing destination file operand", BIOS_RED);
    }
    else {
        uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
        updateDirectoryTable(search_directory_number);

        index = 0;
        buf_len = strlen(source);
        bool valid = false;
        const uint8_t *buf2 = (const uint8_t*) source;
        for (int i = 0; i < buf_len; i++) {
            if (source[i] == '.'){ 
                index = i;
                valid = true;
                splitfirst(buf2, srcName, index);
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

        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest reqsrc = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = search_directory_number,
            .buffer_size = 4 * CLUSTER_SIZE,
        };

        memcpy(&(reqsrc.name), srcName, 8);

        if(valid){
            int32_t retcode;

            splitsecond(buf2, srcExt, index+1,buf_len);
            memcpy(&(reqsrc.ext), srcExt, 3);
            interrupt(0, (uint32_t) &reqsrc, (uint32_t) &retcode, 0x0);

            if (retcode != 0 ) {
                switch (retcode) {
                    case 1:
                        print("cp : Is a directory\n", BIOS_RED);
                        break;
                    case 2:
                        print("cp : Buffer size is not enough\n", BIOS_RED);
                        break;
                    case 3:
                        print("cp : No such file or directory\n", BIOS_RED);;
                        break;
                }
            } else{
                index = 0;
                buf_len = strlen(dest);
                for (int i = 0; i < buf_len; i++) {
                    if (dest[i] == '.'){ 
                        index = i;
                        splitname(dest, dstName, destExt, index+1);
                        break;
                    }
                }
                struct ClusterBuffer cbuf = {0};
                struct FAT32DriverRequest destReq = {
                    .buf = &cbuf,
                    .name = "\0\0\0\0\0\0\0\0",
                    .ext = "\0\0\0",
                    .parent_cluster_number = search_directory_number,
                    .buffer_size = CLUSTER_SIZE
                };
                memcpy(&(destReq.name), dstName, 8);
                memcpy(&(destReq.ext), destExt, 3);
                interrupt(3, (uint32_t) &destReq, (uint32_t) &retcode, 0x0);
                interrupt(2, (uint32_t) &destReq, (uint32_t) &retcode, 0x0);

                if (retcode != 0) {
                    switch (retcode) {
                    case 1:
                        print("cp: cannot copy\n", BIOS_RED);
                        return;
                    case -1:
                        print("cp: Unknown error occured\n", BIOS_RED);
                        return;
                    }
                }
            }
        } else{
            print("cp : No such file or directory\n", BIOS_RED);
        }
        
    }
}

void rm(char *fileName){
    uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    updateDirectoryTable(search_directory_number);


    int index = 0;
    int buf_len = strlen(fileName);
    for (int i = 0; i < buf_len; i++) {
        if (fileName[i] == '.'){ 
            index = i;
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

    int32_t retcode;

    struct ClusterBuffer cl           = {0};
    struct FAT32DriverRequest request = {
        .buf = &cl,
        .name = "\0\0\0\0\0\0\0",
        .ext = "\0\0\0",

        .parent_cluster_number = search_directory_number,
        .buffer_size = 4 * CLUSTER_SIZE,
    };
    
    memcpy(&(request.name), srcName, 8);
    memcpy(&(request.ext), srcExt, 3);
    interrupt(3, (uint32_t) &request, (uint32_t) &retcode, 0x0);

    if (retcode != 0 ) {
        switch (retcode) {
        case 1:
            print("rm : File not found\n", BIOS_RED);
            break;
        default:
            print("rm : No such file or directory\n", BIOS_RED);
            break;
        }
    } 
    
    
}