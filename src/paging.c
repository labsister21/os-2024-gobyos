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

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false
    },
    // TODO: Initialize page manager state properly
};


void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}

/* --- Memory Management --- */
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    uint32_t frame_count = amount / PAGE_FRAME_SIZE;
    uint32_t free_frames = 0;

    // Count free frames
    for (uint32_t i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
        if (!page_manager_state.page_frame_map[i]) {
            ++free_frames;
            if (free_frames >= frame_count) {
                return true; // Enough free memory available
            }
        }
    }
    
    return false; // Insufficient free memory
}

bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */ 
    // Find a free physical frame
    for (uint32_t i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
        if (!page_manager_state.page_frame_map[i]) {
            // Mark the frame as used
            page_manager_state.page_frame_map[i] = true;

            // Update page directory with user flags
            struct PageDirectoryEntryFlag flags = {
                .present_bit = 1,
                .read_write = 1,
                .user_supervisor = 1, // Assuming user space
                .page_size = 1
                // Add other flags as needed
            };

            // Update page directory entry
            update_page_directory_entry(page_dir, (void*)(i * PAGE_FRAME_SIZE), virtual_addr, flags);

            return true; // Allocation successful
        }
    }
    
    return false; // No free frame available
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */
        // Find the virtual address in the page directory
    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;
    
    // Check if the page is currently allocated
    if (page_dir->table[page_index].flag.present_bit) {
        // Get the physical address of the frame
        uint32_t physical_addr = (page_dir->table[page_index].lower_address << 22);

        // Calculate the index of the physical frame
        uint32_t frame_index = physical_addr / PAGE_FRAME_SIZE;

        // Mark the frame as free
        page_manager_state.page_frame_map[frame_index] = false;

        // Clear the page directory entry
        page_dir->table[page_index].flag.present_bit = 0;
        
        return true; // Deallocation successful
    }
    
    return false; // Page not allocated
}

