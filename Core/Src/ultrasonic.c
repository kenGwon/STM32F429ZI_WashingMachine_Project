#include "ultrasonic.h"

extern volatile uint32_t TIM10_10ms_counter_ultrasonic;
extern volatile int32_t ultrasonic_distance;
extern volatile uint8_t one_cycle_capture_finish_flag;

volatile uint8_t ultrasonic_print_flag = 1; // 0: ComPortMaster에 미출력, 1: ComPortMaster에 출력

/*
 * desc: 1.5초마다 HC-SR04 초음파센서로부터 편도거리 데이터를 가져온다.
 */
void Ultrasonic_Processing(void)
{
	int32_t distance;
	uint8_t lcd_buff[20];

	if (TIM10_10ms_counter_ultrasonic >= 150)
	{
		TIM10_10ms_counter_ultrasonic = 0;
		Ultrasonic_Trigger();

		if (one_cycle_capture_finish_flag)
		{
			one_cycle_capture_finish_flag = 0;
			distance = ultrasonic_distance; // 전역변수 값을 바로 쓰지 않고 지역변수에 복사하여 사용하는 것임
			distance = distance * 0.034 / 2; // 1us마다 0.034cm를 이동한다. 그리고 초음파 센서가 리턴한 distance값은 왕복 거리이기 때문에 /2를 해서 편도 거리를 얻어낸다.

			if (ultrasonic_print_flag)
			{
				printf("distance_lv: %dcm\n", distance);
				sprintf(lcd_buff, "distance: %dcm", distance);
				Move_Cursor(1,0);
				LCD_String(lcd_buff);
			}
		}
	}
}

/*
 * desc: HC-SR04 초음파센서의 데이터 시트에서 정의하고 있는대로 트리거 클락 신호를 생성한다.
 */
void Ultrasonic_Trigger(void)
{
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);
	delay_us(2);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 1);
	delay_us(10);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);
}
