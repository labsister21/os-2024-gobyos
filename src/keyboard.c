#include "header/text/keyboard.h"
#include "header/cpu/interrupt.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"
#include "stdbool.h"

struct KeyboardDriverState KeyboardDriverState = {
    .read_extended_mode = false,
    .keyboard_input_on = false,
    .keyboard_buffer = '\0',
};

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;


const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

/* -- Driver Interfaces -- */

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
    activate_keyboard_interrupt();
    KeyboardDriverState.keyboard_input_on = true;

}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
    KeyboardDriverState.keyboard_input_on = false;
}

// Get keyboard buffer value and flush the buffer - @param buf Pointer to char buffer
void get_keyboard_buffer(char *buf){
    *buf = KeyboardDriverState.keyboard_buffer;

    // Flush the keyboard buffer
    KeyboardDriverState.keyboard_buffer = '\0';
}

/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 */

void keyboard_isr(void){
    uint8_t scancode = in(KEYBOARD_DATA_PORT);

    if (KeyboardDriverState.keyboard_input_on) {
        char ascii_char = keyboard_scancode_1_to_ascii_map[scancode];
        
        // Handle special ASCII characters
        if (ascii_char == '\n') {
            // Move the cursor to the beginning of the next line
            cursor_row++;
            cursor_col = 0;  // Reset column to 0 for the next line
            framebuffer_set_cursor(cursor_row, cursor_col);
        } else if (ascii_char == '\b') {
            // Move the cursor back one position
            if (cursor_col > 0) {
                cursor_col--;
                framebuffer_write(cursor_row, cursor_col, ' ', 0x07, 0x00);
                framebuffer_set_cursor(cursor_row, cursor_col);
            }
        } else if (ascii_char == '\t') {
            // Move the cursor to the next tab stop
            for (int i = 0; i < 5; i++) {
              if (cursor_col > 79) {
                cursor_col = 0;
                cursor_row++;
              }
              framebuffer_write(cursor_row, cursor_col, ' ', 0x07, 0x00);
              cursor_col++;
              framebuffer_set_cursor(cursor_row, cursor_col);
            }
        } else {
            // Regular character, update cursor position
            framebuffer_write(cursor_row, cursor_col, ascii_char, 0x07, 0x00);
            cursor_col++;
            // Check if cursor reaches end of line
            if (cursor_col > 79) {
                cursor_row++;
                cursor_col = 0;  // Reset column to 0 for the next line
            }
            framebuffer_set_cursor(cursor_row, cursor_col);
        }
        KeyboardDriverState.keyboard_buffer = ascii_char;
    }

    pic_ack(IRQ_KEYBOARD);
}
