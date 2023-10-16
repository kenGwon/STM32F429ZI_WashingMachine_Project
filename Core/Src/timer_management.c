#include <timer_management.h>

extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

volatile uint32_t TIM10_10ms_counter_ledbar = 0; // for ledbar.c
volatile uint32_t TIM10_10ms_counter_DHT11 = 0; // for DHT11.c
volatile uint32_t TIM10_10ms_counter_ultrasonic = 0; // for ultrasonic.c
volatile uint32_t TIM10_10ms_fan_auto_mode_remain_time_decrease = 0; // for fan_machine.c
volatile uint32_t TIM10_10ms_fan_rotate_direction_led = 0; // for fan_machine.c
volatile uint32_t TIM10_10ms_fan_lcd_display_clear = 0; // for fan_machine.c
volatile uint32_t TIM10_10ms_dcmotor_activate_time = 0; // for dcmotor.c

volatile uint32_t TIM10_10ms_WM_wash_timer = 0;
volatile uint32_t TIM10_10ms_WM_rinse_timer = 0;
volatile uint32_t TIM10_10ms_WM_spin_timer = 0;

volatile int32_t ultrasonic_distance; // for ultrasonic.c // 상승엣지부터 하강엣지까지 펄스가 몇번 카운트 되었는지 그 횟수를 담아둘 전역변수
volatile uint8_t one_cycle_capture_finish_flag = 0; // for ultrasonic.c // 0은 초음파 거리 측정 미완 상태, 1은 초음파 거리 측정 완료 상태 [플래그변수]
volatile uint8_t rising_falling_edge_capture_flag = 0; // for ultrasonic.c // 0은 상승엣지 때문에 콜백 펑션에 들어간 상태, 1은 하강 엣지 때문에 콜백 펑선에 둘어간 상태 [플래그변수]

/*
 * desc: 타이머가 counter period를 채울 때마다 콜백되는 함수이다.
 * param1: 타이머 구조체
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t WM_forward_backward = 0;

	// 1MHz로 분주되어 있는 TIM10이 10000번의 counter period를 채울때마다 콜백되어 변수들을 1씩 증가시킨다.(즉 10ms 타이머 기능을 함)
	if (htim->Instance == TIM10)
	{
		TIM10_10ms_counter_ledbar++;
		TIM10_10ms_counter_DHT11++;
		TIM10_10ms_counter_ultrasonic++;
		TIM10_10ms_fan_auto_mode_remain_time_decrease++;
		TIM10_10ms_fan_rotate_direction_led++;
		TIM10_10ms_fan_lcd_display_clear++;
		TIM10_10ms_dcmotor_activate_time++;

		TIM10_10ms_WM_wash_timer++;
		TIM10_10ms_WM_rinse_timer++;
		TIM10_10ms_WM_spin_timer++;
	}
}

/*
 * desc: HAL_Delay()를 대체하기 위한 함수
 * param1: 딜레이시킬 마이크로 세컨드 시간
 * */
void delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim11, 0);

	while(__HAL_TIM_GET_COUNTER(&htim11) < us) // 파라미터로 들어온 us 시간동안 이 와일문에 잡혀있는 것임
		;
}

/*
 * desc: Input Capture direct mode로 동작하는 타이머가 각 조건에 맞는 완료할때마다 콜백되는 함수이다.
 * param1: 타이머 구조체
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// 초음파 센서의 ECHO핀와 연결되어 있는 TIM6가 상승edge 또는 하강edge 인풋을 캡쳐할 때마다 아래 동작을 실행한다.
	if (htim->Instance == TIM3)
	{
		if (rising_falling_edge_capture_flag == 0) // 상승엣지 때문에 콜백 펑션에 들어온 경우
		{
			__HAL_TIM_SET_COUNTER(htim, 0); // 펄스를 셀 카운터를 초기화 하고 세기 시작하는 것이다.
			rising_falling_edge_capture_flag = 1; // 다음에 콜백 펑선이 불릴 때는 당연히 하강 엣지일 때 일 것이므로 플래그변수를 1로 셋팅해준다.
		}
		else if (rising_falling_edge_capture_flag == 1) // 하강 엣지 때문에 콜백 펑션에 들어온 경우
		{

			rising_falling_edge_capture_flag = 0; // 이제 다음 펄스를 또 카운트 하기 위해서 플래그 변수를 초기화 해준다.
			ultrasonic_distance = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // 상승엣지부터 하강엣지까지 펄스가 몇번 카운트 되었는지 그 값을 읽어온다.
			one_cycle_capture_finish_flag = 1; // 초음파 측정완료
		}
	}
}
