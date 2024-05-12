#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/cpu/interrupt.h"
#include "header/idt.h"
#include "header/text/keyboard.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/memory/paging.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // int col = 0;
    // keyboard_state_activate();
    // while (true) {
    //      char c;
    //      get_keyboard_buffer(&c);
    //      if (c) framebuffer_write(0, col++, c, 0xF, 0);
    // }

    // Allocate first 4 MiB virtual memory
    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    read(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);

    while (true);
}


// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);

//     int col = 0;
//     keyboard_state_activate();
//     while (true) {
//          char c;
//          get_keyboard_buffer(&c);
//          if (c) framebuffer_write(0, col++, c, 0xF, 0);
//     }

//     struct PageDirectory page_directory;
//     void *virtual_address = (void *)0x12345678; 
//     paging_allocate_user_page_frame(&page_directory, virtual_address);

//     // struct BlockBuffer b;
//     // for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
//     // write_blocks(&b, 17, 1);
//     // while (true);
// }