#include "header/text/keyboard.h"
#include "header/cpu/interrupt.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"
#include "stdbool.h"

static bool is_shift = false;
static bool is_capslock = false;

struct KeyboardDriverState keyboard_gobyos = {
    .read_extended_mode = false,
    .keyboard_input_on = false,
    .buffer_index = 0,
    .keyboard_buffer = {'\0'},
};

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;
static uint8_t cursor_col_threshold = 0;


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

const char keyboard_scancode_1_to_ascii_shift[256] = {
      0, 0x1B, '!', '@', '#', '$', '%', '^',  '&', '*', '(',  ')',  '_', '+', '\b', '\t',
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,  '{',  '}',  '\n',   0,   0,   0,
      0,    0,   0,   0,   0,   0,   0, ':', '"', '`',   0,     0,     0,    0,   0,   0,
      0,    0,   0, '<', '>', '?',   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
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
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

const char keyboard_scancode_1_to_ascii_capslock[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',  'O', 'P', '[',  ']', '\n',   0,  'A',  'S',
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0, '\\',  'Z', 'X',  'C',  'V',
    'B',  'N', 'M', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
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

const char* get_scancode_to_ascii_map() {
    if (is_capslock) {
        if (is_shift) {
            return keyboard_scancode_1_to_ascii_shift;
        } else {
            return keyboard_scancode_1_to_ascii_capslock;
        }
    } else {
        if (is_shift) {
            return keyboard_scancode_1_to_ascii_shift;
        } else {
            return keyboard_scancode_1_to_ascii_map;
        }
    }
}

/* -- Driver Interfaces -- */

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
    activate_keyboard_interrupt();
    keyboard_gobyos.keyboard_input_on = true;

}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
    keyboard_gobyos.keyboard_input_on = false;
}

// Get keyboard buffer value and flush the buffer - @param buf Pointer to char buffer
void get_keyboard_buffer(char *buf){
    for (int i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
    buf[i] = keyboard_gobyos.keyboard_buffer[i];
  }
}

bool is_keyboard_blocking(void) {
    return keyboard_gobyos.keyboard_input_on;
}

/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 */

void keyboard_isr(void){
    uint8_t scancode = in(KEYBOARD_DATA_PORT);

    // Check for shift and capslock scancodes
    if (scancode == 0x2A || scancode == 0x36) {
        // If shift (left or right) key is pressed
        is_shift = true;
        pic_ack(IRQ_KEYBOARD);
        return;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        // If shift (left or right) key is pressed
        is_shift = false;
        pic_ack(IRQ_KEYBOARD);
        return;
    } else if (scancode == 0x3A) {
        // If capslock key is pressed or released
        is_capslock = !is_capslock;
        pic_ack(IRQ_KEYBOARD);
        return;
    } 
    
    if (!keyboard_gobyos.keyboard_input_on) {
        keyboard_gobyos.buffer_index = 0;
    } else {
        uint8_t scancode = in(KEYBOARD_DATA_PORT);
        char ascii_char = get_scancode_to_ascii_map()[scancode];

        if (ascii_char != 0) {
          // Handle special ASCII characters
          if (ascii_char == '\n') {
            // newline handler
                if (cursor_row <= 24) {
                    // pindahin kursor ke nextline
                    cursor_row++;
                    cursor_col = 0;  // reset cursor
                }
                keyboard_gobyos.keyboard_buffer[keyboard_gobyos.buffer_index] = '\0';
                keyboard_gobyos.buffer_index = 0;
                framebuffer_set_cursor(cursor_row, cursor_col_threshold);
                keyboard_state_deactivate();
          } else if (ascii_char == '\b') {
            // backspace handler
            // pindahin cursor 1 kebelakang
            if (cursor_col > cursor_col_threshold) {
                cursor_col--;
                framebuffer_write(cursor_row, cursor_col, ' ', 0x07, 0x00);
                framebuffer_set_cursor(cursor_row, cursor_col);
                if(keyboard_gobyos.buffer_index>0){
                    keyboard_gobyos.buffer_index--;
                }
            } else if (cursor_row > cursor_col_threshold) {
                cursor_col = 79;  // pindahin ke previous line (row sblomnya)
                cursor_row--;
                framebuffer_write(cursor_row, cursor_col, ' ', 0x07, 0x00);
                framebuffer_set_cursor(cursor_row, cursor_col);
            }
          } else if (ascii_char == '\t') {
            // tab handler
            for (int i = 0; i < 4; i++) {
                if (cursor_col > 79) {
                    cursor_col = 0;
                    cursor_row++;
                }
                framebuffer_write(cursor_row, cursor_col, ' ', 0x07, 0x00);
                cursor_col++;
                framebuffer_set_cursor(cursor_row, cursor_col);
            }
          } else {
            // check apakah cursor sudah di endline
            if (cursor_col > 79) {
                cursor_col = 0; // reset cursor untuk nextline
                cursor_row++;
            }
            keyboard_gobyos.keyboard_buffer[keyboard_gobyos.buffer_index] = ascii_char;
            keyboard_gobyos.buffer_index++;
            framebuffer_write(cursor_row, cursor_col, ascii_char, 0x07, 0x00);
            cursor_col++;
            framebuffer_set_cursor(cursor_row, cursor_col);
          }
        }
    }
    pic_ack(IRQ_KEYBOARD);
}

void put_char(char c, uint32_t color) {
    if (c != '\n'){
        framebuffer_write(cursor_row, cursor_col, c, color, 0);
        cursor_col += 1;
    } else {
        cursor_row += 1;
        cursor_col = 0;
        framebuffer_set_cursor(cursor_row, cursor_col);
    }
}

void puts_char(char *buf, uint32_t count, uint32_t color) {
    for (uint8_t i = 0; i < count; i++) {
        framebuffer_set_cursor(cursor_row, cursor_col + i + 1);
        if (buf[i] == '\n') {
            cursor_row++;
            cursor_col = 0;
            framebuffer_set_cursor(cursor_row, cursor_col);
        } else {
            framebuffer_write(cursor_row, cursor_col + i, buf[i], color, 0);
            if(i == count - 1) {
                cursor_col = cursor_col + count;
            }
        }
        cursor_col_threshold = cursor_col;
  }
}

void reset_keyboard_position() {
    cursor_row = 0;
    cursor_col = 0;
}
