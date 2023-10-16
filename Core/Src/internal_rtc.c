#include "internal_rtc.h"

extern RTC_HandleTypeDef hrtc;
extern uint8_t lcd_display_mode_flag;

RTC_DateTypeDef my_systemDate = {0}; // date information
RTC_TimeTypeDef my_systemTime = {0}; // time information

// STM32의 RTC로부터 날짜 & 시간 정보를 읽어오는 함수
void get_rtc(void)
{
	static RTC_TimeTypeDef oldTime; // 이전 시각의 정보를 담을 변수 (static을 붙여서 전역처럼 사용)
	char lcd_buff[40];

	HAL_RTC_GetDate(&hrtc, &my_systemDate, RTC_FORMAT_BCD);
	HAL_RTC_GetTime(&hrtc, &my_systemTime, RTC_FORMAT_BCD);

	if (oldTime.Seconds != my_systemTime.Seconds)
	{
		// ComPortMaster에 "YYYY-MM-DD hh:mm:ss" 형태로 뿌려주고 싶음. 그래서 BCD를 십진수로 바꿔주는 함수가 필요해진다.
		// 23년도가 save되어있는 binary format은 001 0111이다. (BCD format이었다면, 0010 0011 이었을 것)
		printf("%04d-%02d-%02d %02d-%02d-%02d\n",
				bcd2dec(my_systemDate.Year)+2000, bcd2dec(my_systemDate.Month), bcd2dec(my_systemDate.Date),
				bcd2dec(my_systemTime.Hours), bcd2dec(my_systemTime.Minutes), bcd2dec(my_systemTime.Seconds)
				);

		if (lcd_display_mode_flag == 0)
		{
			sprintf(lcd_buff, "DATE:%04d-%02d-%02d",
							bcd2dec(my_systemDate.Year)+2000, bcd2dec(my_systemDate.Month), bcd2dec(my_systemDate.Date));
			move_cursor(0, 0);
			lcd_string(lcd_buff);

			sprintf(lcd_buff, "TIME:%02d-%02d-%02d",
							bcd2dec(my_systemTime.Hours), bcd2dec(my_systemTime.Minutes), bcd2dec(my_systemTime.Seconds));
			move_cursor(1, 0);
			lcd_string(lcd_buff);
		}
	}

	oldTime.Seconds = my_systemTime.Seconds;
}

// ComPortMaster에서 "setrtc231016103800"이라는 문자열이 들어오면 이 문자열을 파싱해서 현재 MCU의 RTC 시간을 해당 값으로 셋팅해준다는 것
void set_rtc(char *date_time)
{
	char yy[4], mm[4], dd[4]; // date
	char hh[4], min[4], ss[4]; // time

	strncpy(yy, date_time+6, 2);
	strncpy(mm, date_time+8, 2);
	strncpy(dd, date_time+10, 2);
	strncpy(hh, date_time+12, 2);
	strncpy(min, date_time+14, 2);
	strncpy(ss, date_time+16, 2);

	// ascii --> int --> bcd
	my_systemDate.Year = dec2bcd(atoi(yy));
	my_systemDate.Month = dec2bcd(atoi(mm));
	my_systemDate.Date = dec2bcd(atoi(dd));
	my_systemTime.Hours = dec2bcd(atoi(hh));
	my_systemTime.Minutes = dec2bcd(atoi(min));
	my_systemTime.Seconds = dec2bcd(atoi(ss));

	HAL_RTC_SetDate(&hrtc, &my_systemDate, RTC_FORMAT_BCD);
	HAL_RTC_SetTime(&hrtc, &my_systemTime, RTC_FORMAT_BCD);
}

// 0010 0111 -> 23 (.ioc 환경변수 설정에서 날자값을 bcd포맷으로 저장하게 했기 때문에 이렇게 된다. binary 포맷이었다면 0001 0111 이었을 것)
uint8_t bcd2dec(uint8_t byte)
{
	uint8_t high, low;
	low = byte & 0x0f; // 하위 4bit (low nibble)
	high = (byte >> 4) * 10; // 상위 4bit (high nibble)

	return high + low;
}

// 23 -> 0010 0111
uint8_t dec2bcd(uint8_t byte)
{
	uint8_t high, low;

	high = (byte / 10) << 4;
	low = (byte % 10);

	return high + low;
}

// button0: 시간 정보를 변경하는 버튼. 0시~23시 (up count)
// button1: 분 정보를 변경하는 버튼. 0분~59분 (up count)
// button2: 초 정보를 변경하는 버튼. 0초~59초 (up count)
// button3: 변경 완료 현재까지 변경된 내용을 저장
//
void set_time_button_UI(void)
{
	static uint8_t hour = 0, minute = 0, second = 0;
	char lcd_buff1[20], lcd_buff2[20];

	RTC_TimeTypeDef curr_time;
	RTC_TimeTypeDef temp_time;

	if (lcd_display_mode_flag == 3)
	{
		if(Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
		{
			hour++;
			lcd_command(CLEAR_DISPLAY);
		}
		if(Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
		{
			minute++;
			lcd_command(CLEAR_DISPLAY);
		}
		if(Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
		{
			second++;
			lcd_command(CLEAR_DISPLAY);
		}

		hour %= 24;
		minute %= 60;
		second %= 60;

		HAL_RTC_GetTime(&hrtc, &curr_time, RTC_FORMAT_BCD);
		sprintf(lcd_buff1, "NOW=%02d:%02d:%02d", bcd2dec(curr_time.Hours), bcd2dec(curr_time.Minutes), bcd2dec(curr_time.Seconds));
		move_cursor(0, 0);
		lcd_string(lcd_buff1);

		sprintf(lcd_buff2, "h:%02d m:%02d s:%02d", hour, minute, second);
		move_cursor(1, 0);
		lcd_string(lcd_buff2);

		if(Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
		{
			lcd_command(CLEAR_DISPLAY);

			temp_time.Hours = dec2bcd(hour);
			temp_time.Minutes = dec2bcd(minute);
			temp_time.Seconds = dec2bcd(second);
			HAL_RTC_SetTime(&hrtc, &temp_time, RTC_FORMAT_BCD);

			hour = 0;
			minute = 0;
			second = 0;
			lcd_display_mode_flag = 0;
		}
	}
}
