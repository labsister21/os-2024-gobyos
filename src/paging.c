#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit = 1,
            .flag.read_write = 1,
            .flag.page_size = 1,
            .lower_address = 0,
        },
        [0x300] = {
            .flag.present_bit = 1,
            .flag.read_write = 1,
            .flag.page_size = 1,
            .lower_address = 0,
        },
    }
};

// static struct PageManagerState page_manager_state = {
//     .page_frame_map = {
//         [0]                            = true,
//         [1 ... PAGE_FRAME_MAX_COUNT-1] = false
//     },
//     // TODO: Initialize page manager state properly
// };


// void update_page_directory_entry(
//     struct PageDirectory *page_dir,
//     void *physical_addr, 
//     void *virtual_addr, 
//     struct PageDirectoryEntryFlag flag
// ) {
//     uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
//     page_dir->table[page_index].flag          = flag;
//     page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
//     flush_single_tlb(virtual_addr);
// }

// void flush_single_tlb(void *virtual_addr) {
//     asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
// }



// /* --- Memory Management --- */
// // TODO: Implement
// // bool paging_allocate_check(uint32_t amount) {
// //     // TODO: Check whether requested amount is available
// //     return true;
// // }

// bool paging_allocate_check(uint32_t amount) {
//     // Hitung jumlah frame yang dibutuhkan untuk alokasi
//     uint32_t required_frames = (amount + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE;

//     // Periksa apakah jumlah frame yang dibutuhkan tersedia
//     if (required_frames <= page_manager_state.free_page_frame_count) {
//         // Jika tersedia, kembalikan true
//         return true;
//     } else {
//         // Jika tidak tersedia, kembalikan false
//         return false;
//     }
// }


// // bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
// //     /*
// //      * TODO: Find free physical frame and map virtual frame into it
// //      * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
// //      * - Mark page_manager_state.page_frame_map[]
// //      * - Update page directory with user flags:
// //      *     > present bit    true
// //      *     > write bit      true
// //      *     > user bit       true
// //      *     > pagesize 4 mb  true
// //      */ 
// //     return true;
// // }

// bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
//     // Iterasi melalui page frame map untuk mencari frame fisik yang kosong
//     for (uint32_t i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
//         if (!page_manager_state.page_frame_map[i]) {
//             // Menemukan frame fisik yang kosong, tandai sebagai digunakan
//             page_manager_state.page_frame_map[i] = true;
            
//             // Memperbarui entri pada PageDirectory dengan flag yang sesuai
//             struct PageDirectoryEntryFlag flags = {
//                 .present_bit = 1,
//                 .read_write = 1,
//                 .user_supervisor = 1,
//                 .page_size = 1
//                 // Anda dapat menambahkan flag lain sesuai kebutuhan
//             };

//             // Memperbarui page directory entry
//             update_page_directory_entry(page_dir, (void*)(i * PAGE_FRAME_SIZE), virtual_addr, flags);

//             // Alokasi berhasil, kembalikan true
//             return true;
//         }
//     }

//     // Jika tidak ada frame fisik yang tersedia, kembalikan false
//     return false;
// }

// // bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
// //     /* 
// //      * TODO: Deallocate a physical frame from respective virtual address
// //      * - Use the page_dir.table values to check mapped physical frame
// //      * - Remove the entry by setting it into 0
// //      */
// //     return true;
// // }

// bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
//     // Temukan indeks entri PageDirectory yang sesuai dengan alamat virtual
//     uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    
//     // Periksa apakah entri PageDirectory valid
//     if (page_dir->table[page_index].flag.present_bit) {
//         // Dapatkan alamat fisik dari entri PageDirectory
//         uint32_t physical_addr = (page_dir->table[page_index].higher_address << 22) | (page_dir->table[page_index].lower_address << 12);
        
//         // Hitung indeks frame fisik
//         uint32_t frame_index = physical_addr / PAGE_FRAME_SIZE;
        
//         // Hapus entri pada PageDirectory
//         page_dir->table[page_index].flag.present_bit = 0;
        
//         // Tandai frame fisik sebagai kosong kembali
//         page_manager_state.page_frame_map[frame_index] = false;
        
//         // Alokasi berhasil, kembalikan true
//         return true;
//     }
    
//     // Jika alamat virtual tidak valid, kembalikan false
//     return false;
// }
