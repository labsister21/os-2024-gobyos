
#include "header/clock/clock.h"
#include "header/cmos/cmos.h"
#include "header/cpu/interrupt.h"
#include "header/filesystem/user-shell.h"
#include "header/scheduler/scheduler.h"

uint32_t current_directory = ROOT_CLUSTER_NUMBER;
struct FAT32DirectoryTable dir_table;

void interrupt(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    // so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print(char *buf, uint8_t color){
    interrupt(5, (uint32_t) buf, strlen(buf), color);
}

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

void delay(uint32_t milliseconds) {
    uint32_t count = milliseconds * 10000; // Adjust this multiplier for calibration
    while (count--) {
        __asm__ volatile ("nop");
    }
}



int main (void)
{
    while (1) {
    interrupt(7, 0, 0, 0);
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    read_rtc(&year, &month, &day, &hour, &minute, &second);

    char syear[10];
    char smonth[10];
    char sday[10];
    char shour[10];
    char sminute[10];
    char ssecond[10];
    itoa((int32_t)year, syear);
    itoa((int32_t)month, smonth);
    itoa((int32_t)day, sday);
    itoa((int32_t)hour+7, shour);
    itoa((int32_t)minute, sminute);
    itoa((int32_t)second, ssecond);

    print(syear, BIOS_WHITE);
    print("-",BIOS_WHITE);
    print(smonth, BIOS_WHITE);
    print("-", BIOS_WHITE);
    print(sday, BIOS_WHITE);
    print(" ", BIOS_WHITE);
    print(shour, BIOS_WHITE);
    print(":", BIOS_WHITE);
    print(sminute, BIOS_WHITE);
    print(":", BIOS_WHITE);
    print(ssecond, BIOS_WHITE);

    print(" UTC\n", BIOS_WHITE);
    delay(1000);
    }
  return 0;
}