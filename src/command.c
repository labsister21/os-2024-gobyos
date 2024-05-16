#include "header/filesystem/command.h"
#include "header/filesystem/user-shell.h"
#include "header/stdlib/stdtype.h"

void cat(char *fileName){
    if (strlen(fileName)==0 ) {
        print("cat: missing destination file operand", BIOS_RED);
    }
    else {
    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    
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
 
    int search_directory_number = current_directory;
    int entry_index = findEntryName(srcName);
    if (entry_index != -1) {
        if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
            search_directory_number =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
            updateDirectoryTable(search_directory_number);
        }
    }

    if(valid){
        int32_t retcode;

        struct ClusterBuffer cat           = {0};
        struct FAT32DriverRequest request = {
            .buf = &cat,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",

            .parent_cluster_number = search_directory_number,
            .buffer_size = 4 * CLUSTER_SIZE,
        };
        
        memcpy(&(request.name), srcName, 8);
        memcpy(&(request.ext), srcExt, 3);
        interrupt(0, (uint32_t) &request, (uint32_t) &retcode, 0x0);
        print((char *) &cat, BIOS_BROWN);

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
        } 
    } else{
        print("cat : No such file or directory\n", BIOS_RED);
    }
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

void cp(char* fileName) {
    char source[12];
    char dest[12];

    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};
    char dstName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char destExt[3] = {'\0','\0','\0'};

    int index = 0;
    int buf_len = strlen(fileName);
    const uint8_t *buf2 = (const uint8_t*) fileName;
    for (int i = 0; i < buf_len; i++) {
        if (fileName[i] == ' '){ 
            index = i;
            splitfirst(buf2, source, index+1);
            splitsecond(buf2, dest, index+1,buf_len);
            break;
        }
    }

    if (strlen(fileName)==0 || dest[0]=='\0') {
        print("cp: missing destination file operand", BIOS_RED);
    }
    else {
        updateDirectoryTable(current_directory);

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
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            } else if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            }
        }

        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest reqsrc = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = current_directory,
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
                    .parent_cluster_number = current_directory,
                    .buffer_size = CLUSTER_SIZE
                };

                // hapus dulu file destinasi
                memcpy(&(destReq.name), dstName, 8);
                memcpy(&(destReq.ext), destExt, 3);
                interrupt(3, (uint32_t) &destReq, (uint32_t) &retcode, 0x0);

                // tulis kembali dengan buffer baru
                strcpy(destReq.buf, reqsrc.buf);
                memcpy(&(destReq.name), dstName, 8);
                memcpy(&(destReq.ext), destExt, 3);
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
    if (strlen(fileName)==0) {
        print("rm: missing destination file operand", BIOS_RED);
    }
    else {
        char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
        char srcExt[3] = {'\0','\0','\0'};
        updateDirectoryTable(current_directory);


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
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            } else if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            }
        }

        int32_t retcode;

        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest request = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",

            .parent_cluster_number = current_directory,
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
            } 
        }
    }
}

void ls(){
    updateDirectoryTable(current_directory);
    for (int i = 1; i < 63; i++) {

        if (dir_table.table[i].user_attribute == UATTR_NOT_EMPTY) {
            if (dir_table.table[i].name[7] != '\0') {
                print(dir_table.table[i].name, BIOS_BROWN);
            }
            else {
                print(dir_table.table[i].name, BIOS_BROWN);
            }
            if (dir_table.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[i].ext) != 0) {
                print(".", BIOS_BROWN);
                print(dir_table.table[i].ext, BIOS_BROWN);
            }
            print("\n", BIOS_BROWN);
        }
    }
    if (dir_table.table[63].user_attribute == UATTR_NOT_EMPTY) {
        if (dir_table.table[63].name[7] != '\0') {
            print(dir_table.table[63].name, BIOS_BROWN);
        }
        else {
            print(dir_table.table[63].name, BIOS_BROWN);
        }
        if (dir_table.table[63].attribute != ATTR_SUBDIRECTORY && strlen(dir_table.table[63].ext) != 0) {
            print(".", BIOS_BROWN);
            print(dir_table.table[63].ext, BIOS_BROWN);
        }
        print("\n", BIOS_BROWN);
    }
}

void mkdir(char *fileName){
    if (strlen(fileName)==0 ) {
        print("mkdir: missing destination file operand", BIOS_RED);
    }else {
        int curr = current_directory;
        updateDirectoryTable(current_directory);

        int entry_index = findEntryName(fileName);
        if (entry_index != -1) {
            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            } 
        }

        struct FAT32DriverRequest reqsrc = {
            .buf = 0,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = current_directory,
            .buffer_size = 0,
        };

        memcpy(&(reqsrc.name), fileName, 8);
        current_directory = curr;
        updateDirectoryTable(curr);
        int32_t retcode;
        interrupt(2, (uint32_t) &reqsrc, (uint32_t) &retcode, 0x0);

        if (retcode != 0 ) {
            switch (retcode) {
                case 1:
                    print("mkdir : Directory exist\n", BIOS_RED);
                    break;
            }
        }
    }
}

void cd(char *fileName){
    if (strlen(fileName)==0 ) {
        print("cd: missing destination file operand", BIOS_RED);
    } else{
        int search_directory_number = current_directory;
        updateDirectoryTable(current_directory);

        if (memcmp(fileName, "..", 2) == 0) {
            search_directory_number = (int) ((dir_table.table[0].cluster_high << 16) | dir_table.table[0].cluster_low);
            updateDirectoryTable(search_directory_number);
            return;
        }

        int entry_index = findEntryName(fileName);
        if (entry_index != -1) {
            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            } 
        }

        if(entry_index==-1){
            print("cd : No such file or directory\n", BIOS_RED);
        }

    }
}

void mv(char *fileName){
    char source[12]= {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char dest[12] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};

    char srcName[8] = {'\0','\0','\0','\0','\0','\0','\0','\0'};
    char srcExt[3] = {'\0','\0','\0'};

    int index = 0;
    int buf_len = strlen(fileName);
    const uint8_t *buf2 = (const uint8_t*) fileName;
    for (int i = 0; i < buf_len; i++) {
        if (fileName[i] == ' '){ 
            index = i;
            splitfirst(buf2, source, index+1);
            splitsecond(buf2, dest, index+1,buf_len);
            break;
        }
    }

    if (strlen(fileName)==0 || dest[0]=='\0') {
        print("mv: missing destination file operand", BIOS_RED);
    }
    else {
        int curr = current_directory;
        updateDirectoryTable(current_directory);

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

        if(!valid){
            strcpy(srcName,source);
            valid = true;
        }

        int entry_index = findEntryName(srcName);
        if (entry_index != -1) {
            if (dir_table.table[entry_index].attribute == ATTR_SUBDIRECTORY) {
                current_directory =  (int) ((dir_table.table[entry_index].cluster_high << 16) | dir_table.table[entry_index].cluster_low);;
                updateDirectoryTable(current_directory);
            }
        }

        struct ClusterBuffer cl           = {0};
        struct FAT32DriverRequest reqsrc = {
            .buf = &cl,
            .name = "\0\0\0\0\0\0\0",
            .ext = "\0\0\0",
            .parent_cluster_number = current_directory,
            .buffer_size = 4 * CLUSTER_SIZE,
        };

        memcpy(&(reqsrc.name), srcName, 8);

        if(valid){
            int32_t retcode;

            splitsecond(buf2, srcExt, index+1,buf_len);
            memcpy(&(reqsrc.ext), srcExt, 3);
            interrupt(0, (uint32_t) &reqsrc, (uint32_t) &retcode, 0x0);

            if (retcode == 3 ) {
                print("mv : No such file or directory\n", BIOS_RED);
                
            } else{
                
                int dest_index = findEntryName(dest);
                if (entry_index != -1) {
                    if (dir_table.table[dest_index].attribute == ATTR_SUBDIRECTORY) {
                        current_directory =  (int) ((dir_table.table[dest_index].cluster_high << 16) | dir_table.table[dest_index].cluster_low);;
                        updateDirectoryTable(current_directory);
                    }
                }

                struct ClusterBuffer cbuf = {0};
                struct FAT32DriverRequest destReq = {
                    .buf = &cbuf,
                    .name = "\0\0\0\0\0\0\0\0",
                    .ext = "\0\0\0",
                    .parent_cluster_number = current_directory,
                    .buffer_size = CLUSTER_SIZE
                };

                // tulis ke destinasi
                strcpy(destReq.buf, reqsrc.buf);
                memcpy(&(destReq.name), reqsrc.name, 8);
                memcpy(&(destReq.ext), reqsrc.ext, 3);
                interrupt(2, (uint32_t) &destReq, (uint32_t) &retcode, 0x0);

                if (retcode != 0) {
                    switch (retcode) {
                    case 1:
                        print("mv: cannot copy\n", BIOS_RED);
                        return;
                    case -1:
                        print("mv: Unknown error occured\n", BIOS_RED);
                        return;
                    }
                }

                  // ganti ke directory semula
                current_directory = curr;
                updateDirectoryTable(current_directory);

                // hapus file awal
                interrupt(3, (uint32_t) &reqsrc, (uint32_t) &retcode, 0x0);
            }
        } else{
            print("mv : No such file or directory\n", BIOS_RED);
        }
        }
}
