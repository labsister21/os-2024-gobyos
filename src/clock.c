#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "header/clock/clock.h"
#include "header/cpu/interrupt.h"
#include "header/filesystem/user-shell.h"

void get_time(uint16_t* year, uint16_t* month, uint16_t* day, uint16_t* hour, uint16_t* minute, uint16_t* second) {
    uint16_t timestamp[6];
    interrupt(SYS_READ_RTC_TIME, (uint32_t) timestamp, 0, 0);
    *year = timestamp[0];
    *month = timestamp[1];
    *day = timestamp[2];
    *hour = timestamp[3];
    *minute = timestamp[4];
    *second = timestamp[5];
}