// File : user-shell.h
// Header for user-shell.c, contains the declaration of functions needed to run shell program

#ifndef _USERSHELL_
#define _USERSHELL_

#include "../stdlib/stdtype.h"
#include "../stdlib/stdmem.h"
#include "fat32.h"
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

// Position of current directory
extern uint32_t current_directory;
extern struct FAT32DirectoryTable dir_table;

// Get the parsed args nums and components
int inputparse (char *args_val, int args_info[128][2]);

// Interrupt to main
void interrupt (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Some function using the base interrupt
// Put chars to screen
void print (char* buf, uint8_t color);
void putn(char* buf, uint8_t color, int n);

// Print Current Working Directory
void printCWD (char* path_str, uint32_t current_dir);

/* Functions for pathing */

bool isPathAbsolute(char* args_val, int (*args_info)[2], int args_pos);

void updateDirectoryTable(uint32_t cluster_number);

int findEntryName(char* name);

/* Splash screen */
void screenInit();

#endif