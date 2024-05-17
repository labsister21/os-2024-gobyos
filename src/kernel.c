#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/idt.h"
#include "header/cpu/interrupt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/text/keyboard.h"
#include "header/idt.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/memory/paging.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    // framebuffer_write(3, 8,  'H', 0, 0xF);
    // framebuffer_write(3, 9,  'a', 0, 0xF);
    // framebuffer_write(3, 10, 'i', 0, 0xF);
    // framebuffer_write(3, 11, '!', 0, 0xF);
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

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
    struct ClusterBuffer cbuf[2];

    memcpy(cbuf, "OS matahariku\nOS cintaku\ni love u OS\n", 132);
    request.buf = cbuf;
    clear(&request.name, 8);
    clear(&request.ext, 3);
    memcpy(&request.name, "cintaos", 7);
    memcpy(&request.ext, "txt", 3);
    request.buffer_size = CLUSTER_SIZE;
    write(request);

    clear(&request.name, 8);
    memcpy(cbuf, "os tubes favorit\n", 20);
    memcpy(&request.name, "lain", 4);
    write(request);

    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);

    while (true);
}