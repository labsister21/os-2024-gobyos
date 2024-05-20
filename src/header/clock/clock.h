#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

// Define the system call number for reading RTC time
#define SYS_READ_RTC_TIME 0x30

// Function to get the current time from RTC
void get_time(uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour, uint16_t* minute, uint16_t* second);

#endif /* CLOCK_H */