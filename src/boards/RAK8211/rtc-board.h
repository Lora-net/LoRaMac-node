#ifndef _RTC_H
#define _RTC_H

#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"

typedef void (*rtc_wakeup_callback_t)(void);

void RtcInit(void);

uint32_t rtc_get_timestamp(void);
void rtc_update_timestamp(uint32_t);
void rtc_test_overflow(void);
ret_code_t rtc_schedule_wakeup(uint32_t);

#endif /* _RTC_H */
