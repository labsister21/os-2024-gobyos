#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/filesystem/fat23.h"

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

struct FAT32DriverState driver;

uint32_t cluster_to_lba(uint32_t cluster) {
    return cluster * CLUSTER_BLOCK_COUNT;
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    write_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    read_blocks(ptr, cluster_to_lba(cluster_number), CLUSTER_BLOCK_COUNT*cluster_count);
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
}

bool is_empty_storage(void) {
}

void create_fat32(void) {
}

void initialize_filesystem_fat32(void) {
}

/* Additional Operation */

uint32_t divceil(uint32_t pembilang, uint32_t penyebut){
  uint32_t cmp = pembilang / penyebut;

  if (pembilang % penyebut == 0)
    return cmp;

  return cmp + 1;
}

/* -- CRUD Operation -- */

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
    read_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *t = driver.dir_table_buf.table;

    // periksa apakah direktory kosong atau tidak
    if (t->user_attribute != UATTR_NOT_EMPTY) { 
        return -1; // kosong
    }

    // for loop tiap entri
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++) {
        if (strcmp(t[i].name, request.name)!=0){ // nama sesuai dengan request
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
    read_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);

    struct FAT32DirectoryEntry *t = driver.dir_table_buf.table;

    // periksa direktori kosong atau tidak
    if (t->user_attribute != UATTR_NOT_EMPTY) { 
        return -1;
    }

    // for loop semua entri di direktori
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) ; i++) {
        // cek apakah nama file dan direktori sama
        if (strcmp(t[i].name, request.name)!=0 && (strcmp(t[i].ext,request.ext)!=0)){
            if (t[i].attribute == 1) { // merupakan direktori
                return 1;
            }
            else if(t[i].filesize > request.buffer_size){ // buffer tidak cukup
                return 2;
            }
            else{
                int cnt=0;
                int cluster_num = (driver.dir_table_buf.table[i].cluster_high << 16) + driver.dir_table_buf.table[i].cluster_low;

                // buffer cukup dan salin isi file ke buffer
                // looping selama belum mencapai end of file
                while(cluster_num != FAT32_FAT_END_OF_FILE) {
                    read_clusters(request.buf + CLUSTER_SIZE * cnt, cluster_num, 1);
                    cnt++;
                    cluster_num = driver.fat_table.cluster_map[cluster_num];
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
    read_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);

    // invalid parent class
    if (!(driver.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY && driver.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY)) {
        return 2;
    }

    struct FAT32DirectoryEntry *table = driver.dir_table_buf.table;

    // hitung jumlah cluster yang diperlukan untuk menyimpan data file
    uint16_t count = divceil(request.buffer_size, CLUSTER_SIZE);
    if (count <= 1) count = 1;

    uint16_t currcluster = 3; // cluster yang digunakan
    uint16_t location = -999; // lokasi cluster kosong
    uint16_t loc[count]; // menyimpan lokasi cluster yang kosong
    uint16_t j = 0;

    // for loop semua entri dalam direktori
    for (uint8_t i = 1; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++){
        if (table[i].user_attribute == UATTR_NOT_EMPTY){
            // cluster sudah dipakai
            if ((request.buffer_size == 0 && table[i].attribute == 1) || (request.buffer_size != 0 && !strcmp(table[i].ext, request.ext))){
                return 1;
            }
        }
        else{
            while (j < count && currcluster < 512){ // cluster masih kosong
                if (driver.fat_table.cluster_map[currcluster] == 0){
                    loc[j] = currcluster;
                    j++;
                }
                currcluster++;
            }
            location = i; // catat lokasi saat ketemu cluster kosong
            break;
        }
    }

    if (count > j) return -1; // jumlah cluster yang dibutuhkan melebihi kapasitas

    // Salin informasi file atau direktori baru ke dalam entri direktori
    memcpy(table[location].name, request.name, 8);
    table[location].cluster_low = loc[0];
    table[location].cluster_high = (loc[0] >> 16);
    table[location].user_attribute = UATTR_NOT_EMPTY; // dibuat jadi terisi

    if (request.buffer_size == 0){ // bikin folder
        table[location].attribute = 1;
        init_directory_table(request.buf, request.name, loc[0]); // Inisialisasi tabel direktori baru
    }
    else{ // bikin file
        table[location].attribute = 0; // catat sebagai file
        table[location].filesize = request.buffer_size; // simpan ukuran
        memcpy(table[location].ext, request.ext, 3);

        // Tulis data file ke dalam cluster-cluster yang tersedia
        for (uint16_t i = 1; i <= count; i++){
            write_clusters(request.buf + i * CLUSTER_SIZE, loc[i], 1);
            if (i == count){
                driver.fat_table.cluster_map[loc[i-1]] = FAT32_FAT_END_OF_FILE; // akhir file
            }
            else{
                driver.fat_table.cluster_map[loc[i-1]] = loc[i];
            }
        }
        write_clusters(&driver.fat_table, 1, 1);
    }

    // tulis kembali entri direktori setelah menambahkan file atau direktori baru
    write_clusters(table, request.parent_cluster_number, 1);

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
    read_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);

    // invalid parent class
    if (!(driver.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY && driver.dir_table_buf.table[0].attribute == ATTR_SUBDIRECTORY)) { 
        return -1;
        }

    struct FAT32DirectoryEntry *table = driver.dir_table_buf.table;

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
                    write_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);

                    // hapus semua cluster yang digunakan oleh file
                    for (uint16_t i = 0; i < cnt; i++){
                        loc[i] = currcluster;
                        currcluster = driver.fat_table.cluster_map[currcluster];
                    }

                    for (uint16_t i = 0; i < cnt; i++){
                        driver.fat_table.cluster_map[loc[i]] = 0;
                    }

                    // tulis ulang tabel FAT setelah menghapus file
                    write_clusters(&driver.fat_table, 1, 1);
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
                    driver.fat_table.cluster_map[table[i].cluster_low] = 0;
                    table[i].user_attribute = !UATTR_NOT_EMPTY;
                    table[i].undelete = true;

                    // tulis ulang entri direktori dan tabel FAT setelah menghapus direktori
                    write_clusters(&driver.dir_table_buf, request.parent_cluster_number, 1);
                    write_clusters(&driver.fat_table, 1, 1);
                    return 0; // success
                }
            }
    }

    return 1; // not found 
}
