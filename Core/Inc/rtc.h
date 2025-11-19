#ifndef RTC_H
#define RTC_H

#include "stm32f439xx.h"

#define RTC_WRITE_PROTECTION_UNLOCK_1 0xCAU
#define RTC_WRITE_PROTECTION_UNLOCK_2 0x53U

enum DoW {
    Monday      = 1,
    Tuesday     = 2,
    Wednesday   = 3,
    Thursday    = 4,
    Friday      = 5,
    Saturday    = 6,
    Sunday      = 7
};

typedef struct ts {
    uint8_t secs;
    uint8_t mins;
    uint8_t hours;
    uint8_t date;       // day of month
    uint8_t month;
    uint16_t year;
    uint8_t dayOfWk;        // day name
    uint8_t isDst;

} ts;

void RTC_init(ts *ts);
void RTC_setTime(ts *ts);
void RTC_getTime(ts *ts);
void RTC_checkDst(ts *ts);

#endif