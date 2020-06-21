#include <lib.h>
#include "time.h"

struct tm
{
    int tm_sec;   /* 0-60 */
    int tm_min;   /* 0-59 */
    int tm_hour;  /* 0-23 */
    int tm_mday;  /* 1-31 */
    int tm_mon;   /* 0-11 */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* 0-6 */
    int tm_yday;  /* 0-365 */
    int tm_isdst; /* >0 DST, 0 no DST, <0 information unavailable */
};

static uint8_t getRTC(int reg) 
{
    outb(0x70, reg);
    return inb(0x71);
}

void date(struct tm *date)
{
    int century = getRTC(0x32);
    date->tm_sec = getRTC(0x00);
    date->tm_min = getRTC(0x02);
    date->tm_hour = getRTC(0x04);
    date->tm_mday = getRTC(0x07);
    date->tm_mon = getRTC(0x08) - 1;
    date->tm_year = getRTC(0x09) + (century * 100 - 1900);
    date->tm_wday = getRTC(0x06) - 1;
}