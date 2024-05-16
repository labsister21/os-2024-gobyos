#include <stdint.h>
#include <stdbool.h>
#include "header/filesystem/fat32.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

/* struct to save the file system driver state */
struct FAT32DriverState driverState;

uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster * CLUSTER_BLOCK_COUNT;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    /*index filealllocation tabel mewakili cluster index, 
    cluster 0 : cluster_0_value;
    cluster 1 : cluster_1_value -->  CRUD operation in this section
    cluster 2 : root
    cluster 3 : empty */
    struct FAT32DirectoryEntry dirEntry = {
        .name = {name[0], name[1], name[2], name[3], name[4], name[5], name[6], name[7]},
        .attribute = ATTR_SUBDIRECTORY,
        .user_attribute = UATTR_NOT_EMPTY,
        .cluster_high = parent_dir_cluster >> 16,
        .cluster_low = parent_dir_cluster,
        .filesize = 0,
    };
    // put the entry as the first entry
    dir_table->table[0] = dirEntry;
}

bool is_empty_storage(void){
    /* initiate buffer that containt boot sector*/
    uint8_t temp[BLOCK_SIZE];
    /* read dan put temp to buffer sector boot*/
    read_blocks(temp,BOOT_SECTOR,1);
    /* compare, if buffer containt not equal fs_signature return true*/
    return memcmp(temp, fs_signature, BLOCK_SIZE) != 0;
}

void create_fat32(void) {
    // write the file system signature to the boot sector (cluster 0)
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    // initialize and write FAT to cluster 1
    driverState.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driverState.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driverState.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    write_clusters(driverState.fat_table.cluster_map, 1, 1);
    
    // initialize root directory and write it to cluster 2
    struct FAT32DirectoryTable rootDir = {
        .table = {
            {
            .name = {'r','o','o','t'},
            .attribute = ATTR_SUBDIRECTORY,
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = 0x00,
            .cluster_low = 0x02,
            .filesize = 0 
            }
        }
    };
    write_clusters(rootDir.table, 2, 1);
}


void initialize_filesystem_fat32(void){
    // if empty storage, create new file system, else load the FAT to driverState
    if (is_empty_storage()) {
        create_fat32();
    } 
    else {
        read_clusters(&driverState.fat_table, 1, 1);
    }
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}


/* Operation */

uint32_t divceil(uint32_t pembilang, uint32_t penyebut){
  uint32_t cmp = pembilang / penyebut;

  if (pembilang % penyebut == 0)
    return cmp;

  return cmp + 1;
}
/*-----------------------------------------------------------------------------------*/
/*---------------------------------CRUD OPERATION------------------------------------*/

/**
 *  FAT32 Folder / Directory read
 *
 * @param request buf point to struct FAT32DirectoryTable,
 *                name is directory name,
 *                ext is unused,
 *                parent_cluster_number is target directory table to read,
 *                buffer_size must be exactly sizeof(struct FAT32DirectoryTable)
 * @return Error code: 0 success - 1 not a folder - 2 not found - -1 unknown
 */
int8_t read_directory(struct FAT32DriverRequest request){
    // baca cluster yang berisi directory
    read_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *t = driverState.dir_table_buf.table;

    // periksa apakah direktory kosong atau tidak
    if (t->user_attribute != UATTR_NOT_EMPTY) { 
        return -1; // kosong
    }

    // for loop tiap entri
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++) {
        if (strcmp(t[i].name, request.name)==0){ // nama sesuai dengan request
            if (t[i].attribute == 1) { // entri adalah folder
                // salin cluster pertama ke request buffer untuk mencatat buffer folder
                read_clusters(request.buf, t[i].cluster_low, 1);
                return 0; //success
            }
            return 1; // entri bukan folder
        }
    }
    // tidak ada nama folder yang sesuai
    return 2;
}


/**
 * FAT32 read, read a file from file system.
 *
 * @param request All attribute will be used for read, buffer_size will limit reading count
 * @return Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
 */
int8_t read(struct FAT32DriverRequest request){
    // baca cluster yang berisi directory
    read_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *t = driverState.dir_table_buf.table;

    // periksa direktori kosong atau tidak
    if (t->user_attribute != UATTR_NOT_EMPTY) { 
        return -1;
    }

    read_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // for loop semua entri di direktori
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++) {
        // cek apakah nama file dan direktori sama
        if (memcmp(driverState.dir_table_buf.table[i].name, request.name, 8) == 0 &&
            memcmp(driverState.dir_table_buf.table[i].ext, request.ext, 3) == 0){
            if (t[i].attribute == 1) { // merupakan direktori
                return 1;
            }
            else if(t[i].filesize > request.buffer_size){ // buffer tidak cukup
                return 2;
            }
            else{
                int cnt=0;
                int cluster_num = (driverState.dir_table_buf.table[i].cluster_high << 16) + driverState.dir_table_buf.table[i].cluster_low;

                // buffer cukup dan salin isi file ke buffer
                // looping selama belum mencapai end of file
                while(cluster_num != FAT32_FAT_END_OF_FILE) {
                    read_clusters(request.buf + CLUSTER_SIZE * cnt, cluster_num, 1);
                    cnt++;
                    cluster_num = driverState.fat_table.cluster_map[cluster_num];
                }
                
                return 0; //success
            }
            
        }
    }
    return 3; // file not found
}


/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request){
    // baca cluster yang berisi directory
    read_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *table = driverState.dir_table_buf.table;

    // invalid parent cluster
    if (!(table[0].user_attribute == UATTR_NOT_EMPTY && table[0].attribute == ATTR_SUBDIRECTORY)) {
        return 2;
    }

    int entryRow = 0;

    // cek apakah sudah ada file / folder dengan nama sama
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        if (memcmp(table[i-1].name, &request.name, 8) == 0) {
            if (memcmp(&request.ext, table[i-1].ext, 3) == 0) {
                return 1;
            } 
            if (request.ext[0] == '\0' && table[i-1].ext[0] == '\0') {
                return 1;
            }
        }
        if (driverState.dir_table_buf.table[i-1].user_attribute != UATTR_NOT_EMPTY ) {
            entryRow = i-1;
        }
    }

    // tidak ada nama folder/file yang sama lanjut tulis
    read_clusters(&driverState.fat_table, FAT_CLUSTER_NUMBER, 1);
    
    table[request.parent_cluster_number].user_attribute = UATTR_NOT_EMPTY;

    if (request.buffer_size == 0){ // bikin folder

        uint32_t clusterNumber = 0x0;

        // cari lokasi cluster yang kosong
        while (driverState.fat_table.cluster_map[clusterNumber] != FAT32_FAT_EMPTY_ENTRY 
        && clusterNumber < 0x800) {
            clusterNumber++;
        }

        // kalau gaada yang kosong return -1
        if (clusterNumber == 0x800) { return -1; }

        // catat end of file baru
        driverState.fat_table.cluster_map[clusterNumber] = FAT32_FAT_END_OF_FILE;
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // update parent directory table 
        struct FAT32DirectoryEntry dirEntry = {
            .name = {request.name[0], request.name[1], request.name[2], request.name[3], request.name[4], request.name[5], request.name[6], request.name[7]},
            .ext = {request.ext[0], request.ext[1], request.ext[2]},
            .attribute = ATTR_SUBDIRECTORY,
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = clusterNumber >> 16,
            .cluster_low = clusterNumber,
            .filesize = request.buffer_size
        };
        driverState.dir_table_buf.table[entryRow] = dirEntry;
        write_clusters(driverState.dir_table_buf.table, request.parent_cluster_number, 1);

        // buat directory baru
        read_clusters(driverState.dir_table_buf.table, clusterNumber, 1);

        dirEntry.cluster_high = request.parent_cluster_number >> 16;
        dirEntry.cluster_low = request.parent_cluster_number;
        driverState.dir_table_buf.table[0] = dirEntry;
        write_clusters(driverState.dir_table_buf.table, clusterNumber, 1);
    }
    else{ // bikin file
    
        int cnt = request.buffer_size % CLUSTER_SIZE;
        int clustercount = request.buffer_size / CLUSTER_SIZE;

        // ganjil tambah satu cluster
        if (cnt != 0) { clustercount++; } 

        // cari lokasi cluster pertama yang kosong
        int avail = 0;
        int start = 0;
        for (int i = 0;i<=0x800;i++) {
            if (i==0x800){
                return -1;
            }
            if (driverState.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY) {
                avail++;
                start = i;
                break;
            }
        }

        // cari cluster kosong lainnya
        uint32_t prevClusterNumber = start;
        uint32_t currClusterNumber = start + 1;
        while (currClusterNumber < 0x800 && avail < clustercount) {
            if (driverState.fat_table.cluster_map[currClusterNumber] == FAT32_FAT_EMPTY_ENTRY) {
                driverState.fat_table.cluster_map[prevClusterNumber] = currClusterNumber;
                prevClusterNumber = currClusterNumber;
                avail++;
            }
            currClusterNumber++;
        }

        // cluster yang tersisa tidak cukup return error
        if (avail != clustercount) { 
            uint32_t tempClusterNumber;
            // reset ke awal
            while (driverState.fat_table.cluster_map[start] != FAT32_FAT_EMPTY_ENTRY) {
                tempClusterNumber = driverState.fat_table.cluster_map[start];
                driverState.fat_table.cluster_map[start] = FAT32_FAT_EMPTY_ENTRY;
                start = tempClusterNumber;
            }
            return -1;
        }
        else {
            driverState.fat_table.cluster_map[prevClusterNumber] = FAT32_FAT_END_OF_FILE;
        }
        
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

        // update parent directory 
        struct FAT32DirectoryEntry dirEntry = {
            .name = {request.name[0], request.name[1], request.name[2], request.name[3], request.name[4], request.name[5], request.name[6], request.name[7]},
            .ext = {request.ext[0], request.ext[1], request.ext[2]},
            .user_attribute = UATTR_NOT_EMPTY,
            .cluster_high = start >> 16,
            .cluster_low = start,
            .filesize = request.buffer_size
        };
        table[entryRow] = dirEntry;
        write_clusters(table, request.parent_cluster_number, 1);

        // tulis file ke cluster
        int i = 0;
        while (driverState.fat_table.cluster_map[start] != FAT32_FAT_END_OF_FILE) {
            write_clusters((uint8_t*) request.buf + CLUSTER_SIZE*i, start, 1);
            start = driverState.fat_table.cluster_map[start];
            i++;
        }
        write_clusters((uint8_t*) request.buf + CLUSTER_SIZE*i, start, 1);
    }

    return 0; // success


}


/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request){
    // baca cluster yang berisi directory
    read_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);

    // invalid parent class
    if (!(driverState.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY && driverState.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY)) { 
        return -1;
        }

    struct FAT32DirectoryEntry *table = driverState.dir_table_buf.table;

    int entryRow = 0;
    bool found = false;
    bool isDirectory = false;
    uint32_t clusterNumber;

    // cek apakah sebuah directory atau file
    for (int i =2; i <= 64; i++) {
        if (memcmp(table[i-1].name, request.name, 8) == 0 && memcmp(table[i-1].ext, request.ext, 3) == 0 && table[i-1].user_attribute == UATTR_NOT_EMPTY) {
            found = true;
            entryRow = i-1;
            clusterNumber = ((uint32_t) table[i-1].cluster_high) << 16;
            clusterNumber |= (uint32_t) table[i-1].cluster_low;
            if (table[i-1].attribute == ATTR_SUBDIRECTORY) {
                isDirectory = true;
            }
            break;
        }
    }

    // jika file / folder tidak ditemukan
    if (!found) { return 1; }

    if (!isDirectory) {
        // entri adalah file
        for (int i = 0; i < 8; i++) {
            table[entryRow].name[i] = 0;
            if (i < 3) {
                table[entryRow].ext[i] = 0;
            } 
        }
        table[entryRow].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        table[entryRow].attribute = FAT32_FAT_EMPTY_ENTRY;
        write_clusters(table, request.parent_cluster_number, 1);

        // delete semua file dan cluster dari FAT table
        uint32_t currClusterNumber = clusterNumber;
        uint32_t prevClusterNumber = 0;
        while (driverState.fat_table.cluster_map[currClusterNumber] != FAT32_FAT_END_OF_FILE) {
            prevClusterNumber = currClusterNumber;
            currClusterNumber = driverState.fat_table.cluster_map[currClusterNumber];
            driverState.fat_table.cluster_map[prevClusterNumber] = FAT32_FAT_EMPTY_ENTRY;
        }
        driverState.fat_table.cluster_map[currClusterNumber] = 0;
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    }
    else {
        // entry berupa directory
        struct FAT32DirectoryTable tempDir;
        read_clusters(tempDir.table, clusterNumber, 1);

        // check if the directory is empty
        bool empty = true;

        for (int i=2; i <= 64; i++) {
            if (tempDir.table[i-1].user_attribute == UATTR_NOT_EMPTY) {
                empty = false;
            }
        }

        // directory sudah kosong
        if (!empty) { return 2;}

        // hapus file
        for (int i = 0; i < 8; i++) {
            table[entryRow].name[i] = 0;
            if (i < 3) {
                table[entryRow].ext[i] = 0;
            } 
        }
        table[entryRow].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        table[entryRow].attribute = FAT32_FAT_EMPTY_ENTRY;
        write_clusters(table, request.parent_cluster_number, 1);

        // hapus directory
        for (int i = 0; i < 8; i++) {
            tempDir.table[0].name[i] = 0;
            if (i < 3) {
                tempDir.table[0].ext[i] = 0;
            } 
        }
        tempDir.table[0].user_attribute = FAT32_FAT_EMPTY_ENTRY;
        tempDir.table[0].attribute = FAT32_FAT_EMPTY_ENTRY;
        write_clusters(tempDir.table, clusterNumber, 1);

        // hapus direktori di fat
        driverState.fat_table.cluster_map[clusterNumber] = FAT32_FAT_EMPTY_ENTRY;
        write_clusters(driverState.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    }

    return 0;
}