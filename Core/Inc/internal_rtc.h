#ifndef _INTERNAL__RTC_H_
#define _INTERNAL__RTC_H_

#include "main.h"
#include "i2c_lcd.h"
#include "string.h"
#include "stdlib.h"
#include "button.h"

void get_rtc(void);
void set_rtc(char *date_time);
uint8_t bcd2dec(uint8_t byte);
uint8_t dec2bcd(uint8_t byte);
void set_time_button_UI(void);

#endif /*_INTERNAL__RTC_H_*/
