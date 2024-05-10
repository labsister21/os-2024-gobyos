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
    read_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *t = driverState.dir_table_buf.table;

    // periksa direktori kosong atau tidak
    if (t->user_attribute != UATTR_NOT_EMPTY) { 
        return -1;
    }

    // for loop semua entri di direktori
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++) {
        // cek apakah nama file dan direktori sama
        if (strcmp(t[i].name, request.name)==0 && (strcmp(t[i].ext,request.ext)==0)){
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

    // cek apakah sudah ada file / folder dengan nama sama
    for (uint8_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        if (memcmp(table[i].name, &request.name, 8) == 0) {
            if (memcmp(&request.ext, table[i].ext, 3) == 0) {
                return 1;
            } 
            if (request.ext[0] == '\0' && table[i].ext[0] == '\0') {
                return 1;
            }
        }
    }

    // tidak ada nama folder/file yang sama lanjut tulis
    read_clusters(&driverState.fat_table, FAT_CLUSTER_NUMBER, 1);
    
    table[request.parent_cluster_number].user_attribute = UATTR_NOT_EMPTY;

    if (request.buffer_size == 0){ // bikin folder

        // cari akhir dari fat table
        uint32_t end = 0;
        for (int i = 0; i < CLUSTER_MAP_SIZE; i++) {
            if (driverState.fat_table.cluster_map[i] == 0) {
                end = i;
                break;
            }
        }
        // tandai menjadi end of file
        driverState.fat_table.cluster_map[end] = FAT32_FAT_END_OF_FILE;

        for (uint8_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
            struct FAT32DirectoryEntry new;
            // cari lokasi di tabel yang kosong
            if (table[i].name[0] == '\0') {
                memcpy(&new.name, request.name, 8);
                new.attribute = ATTR_SUBDIRECTORY;
                new.cluster_high = end >> 16;
                new.cluster_low = end & 0xFFFF;
                new.undelete = true;
                new.filesize = 0;

                // isi dengan folder baru
                table[i] = new;
                break;
            }
        }

        // buat sub directory
        struct FAT32DirectoryTable sub_dir_table;
        init_directory_table(&sub_dir_table, request.name, request.parent_cluster_number);

        write_clusters(&driverState.fat_table, FAT_CLUSTER_NUMBER, 1);
        write_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);
        write_clusters(&sub_dir_table, end, 1);
    }
    else{ // bikin file
        int prev = 0;
        int cnt = 0;
        int32_t total_bytes = request.buffer_size;
        uint32_t Index = 0;
        uint32_t file_size = 0;
        uint32_t written_data = 0;

        for (int i = 3; i < CLUSTER_MAP_SIZE; i++) {
            // kalau masih cukup lanjut tulis
            if (driverState.fat_table.cluster_map[i] == 0 && total_bytes > 0) {
                if (cnt >= 1) {
                    // catat lokasi
                    driverState.fat_table.cluster_map[prev] = i;
                } else {
                    Index = i;
                }
                
                write_clusters(request.buf + written_data, i, 1);

                // catat pemakaian data
                if(total_bytes>CLUSTER_SIZE){
                    written_data += CLUSTER_SIZE;
                }else{
                    written_data += total_bytes;
                }

                // kurangi pemakaian memori
                total_bytes -= CLUSTER_SIZE;

                // kalau udah penuh catat end of file
                if (total_bytes > 0){
                    prev = i;
                } else {
                    file_size = written_data;
                    driverState.fat_table.cluster_map[prev] = FAT32_FAT_END_OF_FILE;
                    break;
                }
                cnt++;
            }
        }
        

        struct FAT32DirectoryEntry new;

        for (uint8_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
            // cari lokasi yang kosong
            if (table[i].name[0] == '\0') {
                memcpy(&new.name, request.name, 8);
                memcpy(&new.ext, request.ext, 3);
                new.user_attribute = UATTR_NOT_EMPTY;
                new.cluster_high = Index >> 16;
                new.cluster_low = Index & 0xFFFF;
                new.undelete = true;
                new.filesize = file_size;

                table[i] = new;
                break;
            }
        }

        write_clusters(&driverState.fat_table, FAT_CLUSTER_NUMBER, 1);
        write_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);
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

    // for loop semua entri dalam direktori
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++){
            if(table[i].user_attribute == UATTR_NOT_EMPTY){ // entri keisi

                // hitung jumlah cluster yang diperlukan untuk menyimpan data file
                uint16_t cnt = divceil(table[i].filesize, CLUSTER_SIZE);
                uint16_t loc[cnt]; // simpan lokasi entri
                uint16_t currcluster = table[i].cluster_low;

                if(table[i].attribute==0 && strcmp(table[i].ext, request.ext)){ // hapus file
                    // file dikosongin
                    table[i].user_attribute = !(UATTR_NOT_EMPTY);
                    table[i].undelete = true;
                    write_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);

                    // hapus semua cluster yang digunakan oleh file
                    for (uint16_t i = 0; i < cnt; i++){
                        loc[i] = currcluster;
                        currcluster = driverState.fat_table.cluster_map[currcluster];
                    }

                    for (uint16_t i = 0; i < cnt; i++){
                        driverState.fat_table.cluster_map[loc[i]] = 0;
                    }

                    // tulis ulang tabel FAT setelah menghapus file
                    write_clusters(&driverState.fat_table, 1, 1);
                    return 0; // success
                }
            }
            else{
                if (table[i].attribute) { // hapus folder
                    struct FAT32DirectoryTable directory;

                    // baca cluster yang berisi direktori yang akan dihapus
                    read_clusters(&directory, table[i].cluster_low, 1);
                    // periksa apakah direktori kosong atau tidak
                    for (uint16_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++){
                        if (directory.table[i].user_attribute == UATTR_NOT_EMPTY){
                            return 2; // direktori tidak kosong dan tidak dapat dihapus
                        }
                    }

                    // hapus semua cluster yang digunakan oleh direktori
                    driverState.fat_table.cluster_map[table[i].cluster_low] = 0;
                    table[i].user_attribute = !UATTR_NOT_EMPTY;
                    table[i].undelete = true;

                    // tulis ulang entri direktori dan tabel FAT setelah menghapus direktori
                    write_clusters(&driverState.dir_table_buf, request.parent_cluster_number, 1);
                    write_clusters(&driverState.fat_table, 1, 1);
                    return 0; // success
                }
            }
    }

    return 1; // not found 
}