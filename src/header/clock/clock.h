#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#include "../stdlib/stdtype.h"
#include "../stdlib/stdmem.h"
#include "../stdlib/string.h"

// Color declarations
#define BIOS_LIGHT_GREEN 0b1010
#define BIOS_GREY        0b0111
#define BIOS_DARK_GREY   0b1000
#define BIOS_LIGHT_BLUE  0b1001
#define BIOS_RED         0b1100
#define BIOS_BROWN       0b0110
#define BIOS_WHITE       0b1111
#define BIOS_BLACK       0b0000
#define BIOS_PINK        0b1101

// Position of current directory
extern uint32_t current_directory;
extern struct FAT32DirectoryTable dir_table;

// Interrupt to main
void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Put chars to screen
void print (char* buf, uint8_t color);

// Define the system call number for reading RTC time
#define SYS_READ_RTC_TIME 0x30

void delay(uint32_t milliseconds);

// Function to get the current time from RTC
void get_time(uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour, uint16_t* minute, uint16_t* second);

#endif /* CLOCK_H */