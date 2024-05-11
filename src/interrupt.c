#include "header/cpu/interrupt.h"
#include "header/text/keyboard.h"
#include "header/cpu/portio.h"
#include "header/text/framebuffer.h"
#include "header/filesystem/fat32.h"



void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    uint8_t a1, a2;

    // Save masks
    a1 = in(PIC1_DATA); 
    a2 = in(PIC2_DATA);

    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();
    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();
    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);

    // Restore masks
    out(PIC1_DATA, a1);
    out(PIC2_DATA, a2);
}

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
    case PAGE_FAULT:
        __asm__("hlt");
        break;
    case PIC1_OFFSET + IRQ_TIMER:
        pic_ack(0);
        break;
    case PIC1_OFFSET + IRQ_KEYBOARD:
        keyboard_isr();
        break;
    case 0x30:
        syscall(frame);
        break;
    }
}

void syscall(struct InterruptFrame frame) {
    if (frame.cpu.general.eax == 0) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) frame.cpu.general.ebx;
        *((int8_t*) frame.cpu.general.ecx) = read(request);
    } else if (frame.cpu.general.eax == 1) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) frame.cpu.general.ebx;
        *((int8_t*) frame.cpu.general.ecx) = read_directory(request);
    } else if (frame.cpu.general.eax == 2) {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) frame.cpu.general.ebx;
        *((int8_t*) frame.cpu.general.ecx) = write(request);
    } else if (frame.cpu.general.eax == 3) {
       struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) frame.cpu.general.ebx;
        *((int8_t*) frame.cpu.general.ecx) = delete(request);
    } else if (frame.cpu.general.eax == 4) {
        keyboard_state_activate();
        __asm__("sti");
        while (is_keyboard_blocking());
        char buf[KEYBOARD_BUFFER_SIZE];
        get_keyboard_buffer(buf);
        memcpy((char*) frame.cpu.general.ebx, buf, frame.cpu.general.ecx);
    } else if (frame.cpu.general.eax == 5) {
        puts_char(
        (char*) frame.cpu.general.ebx,
        frame.cpu.general.ecx,
        frame.cpu.general.edx ); 
    } else if (frame.cpu.general.eax == 6) {
        read_clusters((struct FAT32DirectoryTable*) frame.cpu.general.ebx, frame.cpu.general.ecx, 1);
    } else if (frame.cpu.general.eax == 7) {
        framebuffer_clear();
        reset_keyboard_position();
    } 
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0 = 0x10
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8;
}