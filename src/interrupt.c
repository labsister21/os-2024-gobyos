#include "header/cpu/interrupt.h"
#include "header/cpu/portio.h"

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
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
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        // ...
    }
}

// void activate_keyboard_interrupt(void) {
//     out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
// }

// void main_interrupt_handler(struct InterruptFrame frame) {
//     switch (frame.int_number) {
//         case PIC1_OFFSET + IRQ_TIMER:
//             // Example timer interrupt handling logic
//             // Increment a system tick count, update scheduler, etc.
//             // (In a real system, you would do more here, like updating the system clock or triggering a scheduler)
//             pic_ack(IRQ_TIMER);
//             break;

//         case PIC1_OFFSET + IRQ_KEYBOARD:
//             // Example keyboard interrupt handling logic
//             // Read the keyboard input buffer, process the key press, etc.
//             // (In a real system, you would read from the keyboard's I/O port to retrieve the scan code)
//             pic_ack(IRQ_KEYBOARD);
//             break;

//         // Add cases for other interrupts as needed
//         // For example, handling for serial port, disk, network, etc.

//         default:
//             // Unknown interrupt handling
//             // Log the unknown interrupt, or ignore if it's a spurious interrupt
//             // Acknowledge if necessary to prevent the PIC from locking up
//             if (frame.int_number >= PIC1_OFFSET && frame.int_number < PIC1_OFFSET + 16) {
//                 pic_ack(frame.int_number - PIC1_OFFSET);
//             }
//             break;
//     }
// }

