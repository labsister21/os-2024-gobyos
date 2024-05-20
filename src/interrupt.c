#include "header/cpu/interrupt.h"
#include "header/text/keyboard.h"
#include "header/cpu/portio.h"
#include "header/text/framebuffer.h"
#include "header/filesystem/fat32.h"
#include "header/process/process.h"
#include "header/scheduler/scheduler.h"
#include "header/cmos/cmos.h"
#include "header/clock/clock.h"
#include "header/stdlib/stdmem.h"

#define PIT_MAX_FREQUENCY   1193182
#define PIT_TIMER_FREQUENCY 1000
#define PIT_TIMER_COUNTER   (PIT_MAX_FREQUENCY / PIT_TIMER_FREQUENCY)

#define PIT_COMMAND_REGISTER_PIO          0x43
#define PIT_COMMAND_VALUE_BINARY_MODE     0b0
#define PIT_COMMAND_VALUE_OPR_SQUARE_WAVE (0b011 << 1)
#define PIT_COMMAND_VALUE_ACC_LOHIBYTE    (0b11  << 4)
#define PIT_COMMAND_VALUE_CHANNEL         (0b00  << 6) 
#define PIT_COMMAND_VALUE (PIT_COMMAND_VALUE_BINARY_MODE | PIT_COMMAND_VALUE_OPR_SQUARE_WAVE | PIT_COMMAND_VALUE_ACC_LOHIBYTE | PIT_COMMAND_VALUE_CHANNEL)
#define SYS_READ_RTC_TIME 0x30
#define PIT_CHANNEL_0_DATA_PIO 0x40

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}


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
    } else if (frame.cpu.general.eax == 8) {
        // untuk terminasi proses
        uint32_t pid = *((uint32_t*) frame.cpu.general.ebx);
        process_destroy(pid);
    } else if (frame.cpu.general.eax == 9) {

        struct FAT32DriverRequest request2 = {
            .buf                   = (uint8_t*) 0,
            .name                  = "",
            .ext                   = "\0\0\0",
            .parent_cluster_number = ROOT_CLUSTER_NUMBER,
            .buffer_size           = 0x100000,
        };
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) frame.cpu.general.ebx;
        strcpy(request2.name,request.buf);
        int32_t result = process_create_user_process(request2);
        *((int8_t*) frame.cpu.general.ecx) = result;

    } else if (frame.cpu.general.eax == 10) {
        // untuk mendapat informasi proses
        char ps[200];
        clear(ps,200);
        get_process_metadata(ps);
        strcpy((char*) frame.cpu.general.ebx, ps);
    } else if (frame.cpu.general.eax == 11) {
        // memanggil clock

        *((bool *)frame.cpu.general.ebx) = get_timestamp();
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