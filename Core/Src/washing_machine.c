#include "washing_machine.h"

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern uint8_t rx_data;
extern uint8_t bt_rx_data;

extern volatile uint32_t TIM10_10ms_WM_wash_timer;
extern volatile uint32_t TIM10_10ms_WM_rinse_timer;
extern volatile uint32_t TIM10_10ms_WM_spin_timer;

// Check_Lid_Open()에서 초음파 센서 측정을 위한 전역변수 from timer_management.c
extern volatile uint32_t TIM10_10ms_counter_ultrasonic;
extern volatile uint32_t TIM10_10ms_counter_DHT11;
extern volatile int32_t ultrasonic_distance;
extern volatile uint8_t one_cycle_capture_finish_flag;


volatile uint8_t mode_complete_alarm_stop_start_flag = STOP;

static uint8_t WashingMachine_curr_status = IDLE_MODE;
static RTC_TimeTypeDef old_time = {0};
static RTC_TimeTypeDef current_time = {0}; // time information
static RTC_DateTypeDef current_date = {0}; // date information

static int32_t wash_remain_time = 0;
static int32_t rinse_remain_time = 0;
static int32_t spin_remain_time = 0;
static RTC_TimeTypeDef wash_complete_time = {0};
static RTC_TimeTypeDef rinse_complete_time = {0};
static RTC_TimeTypeDef spin_complete_time = {0};

static void Idle_Mode_Laundry(void);
static void Wash_Mode_Laundry(void);
static void Rinse_Mode_Laundry(void);
static void Spin_Mode_Laundry(void);
static bool Check_Lid_Open(void);
static bool Check_Motor_OverHeat(void);
static void Idle_Mode_Display(void);
static void Wash_Mode_Setting_display(void);
static void Wash_Mode_Running_display(RTC_TimeTypeDef complete_time);
static void Rinse_Mode_Setting_display(void);
static void Rinse_Mode_Running_display(RTC_TimeTypeDef complete_time);
static void Spin_Mode_Setting_display(void);
static void Spin_Mode_Running_display(RTC_TimeTypeDef complete_time);

/*
 * desc: 세탁기 프로그램 구동에 필요한 H/W를 켠다.
 */
void WashingMachine_Init(void)
{
	HAL_UART_Receive_IT(&huart3, &rx_data, 1); // activate interrupt from RX huart3
	HAL_UART_Receive_IT(&huart6, &bt_rx_data, 1); // activate interrupt from RX huart6

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // 세탁기 뚜껑 개폐용 SERVO motor 구동하기 위해 필요
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1); // 세탁기 뚜껑 개폐 상태 확인하는 초음파센서 활용하기 위해 필요
	HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1); // 드럼통 동작용 DC motor 구동하기 위해 필요
	HAL_TIM_Base_Start_IT(&htim10); // 다수의 10ms 타이머 활옹하기 위해 필요
	HAL_TIM_Base_Start_IT(&htim11); // DHT11센서에서 delay_us() 함수가 올바르게 작동해야 하므로 필요
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1); // 세탁, 헹굼, 탈수 동작완료 안내 부저 작동을 위해 필요

	Buzzer_Turn_Off(); // 최초에 부저 소리 안나게 초기화
	I2C_LCD_Init(); // 세탁기 상태 보여주는 LCD 활용하기 위해 필요
	Idle_Mode_Display(); // 최초에 idle 모드 I2C LED 화면 출력 위해 필요
	DHT11_Init(); // 탈수 완료 여부 확인하는 온습도 센서 활용하기 위해 필요
}

/*
 * desc: 세탁기 프로그램 구동에 사용했던 H/W를 끈다.
 */
void WashingMachine_Terminate(void)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_1);
	HAL_TIM_Base_Stop_IT(&htim10);
	HAL_TIM_Base_Stop_IT(&htim11);
}

/*
 * desc: 세탁기 프로그램의 메인 동작을 결정하는 함수이다.
 */
void WashingMachine_Processing(void)
{
	switch (WashingMachine_curr_status)
	{
		case IDLE_MODE:
			Idle_Mode_Laundry();
			break;

		case WASH_MODE:
			Wash_Mode_Laundry();
			break;

		case RINSE_MODE:
			Rinse_Mode_Laundry();
			break;

		case SPIN_MODE:
			Spin_Mode_Laundry();
			break;

		default:
			break;
	}
}

/*
 * desc:
 */
static void Idle_Mode_Laundry(void)
{
	open_WashingMachine_Lid();
	Mode_Complete_Alarm(); // 이걸 여기서도 불르고 있어야 로직이 맞게 작동함..

	DCmotor_Break();
	LEDbar_All_Off();
	FND4digit_off();

	Idle_Mode_Display(); // 아이들 모드일때는 fnd 안쓰고 있기 때문에 led가 핑글핑글 돌면서 시간 표시해줘도 됨

	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		WashingMachine_curr_status++;
		WashingMachine_curr_status %= 4;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Wash_Mode_Setting_display(); // 순환식 구조의 다음인 세탁모드를 출력
	}
}

/*
 * desc: "세탁/헹굼/탈수" 중 "세탁"에 관련된 동작을 실행하는 함수이다.
 *       세탁모드에서 드럼통 모터는 70%의 고정 duty cycle 출력으로 작동한다.
 */
static void Wash_Mode_Laundry(void)
{
	static uint8_t wash_mode_start_stop_flag = STOP;
	static uint8_t dcmotor_forward_backward_flag = FORWARD;


	/************************BEGIN 기본 동작 부분************************/
	close_WashingMachine_Lid();
	Mode_Complete_Alarm();

	// 뚜껑이 열려있으면 동작을 중지하고 idle 화면으로 이동
	if (Check_Lid_Open())
	{
		mode_complete_alarm_stop_start_flag = STOP;
		WashingMachine_curr_status = 0;
		LCD_Command(CLEAR_DISPLAY);
		Idle_Mode_Display();
	}


	if (wash_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();
	}
	else if (wash_mode_start_stop_flag == START && wash_remain_time > 0)
	{
		// 완료부저가 실행중이더라도, 다시 모드가 시작되면 당연히 부저는 꺼져야함
		mode_complete_alarm_stop_start_flag = STOP;

		// 남은시간은 자동으로 1초씩 감소
		if (TIM10_10ms_WM_wash_timer > 100)
		{
			TIM10_10ms_WM_wash_timer = 0;
			wash_remain_time--;
			if (wash_remain_time < 0) {wash_remain_time = 0;}

			if (dcmotor_forward_backward_flag == FORWARD)
			{
				dcmotor_forward_backward_flag = BACKWARD;
			}
			else
			{
				dcmotor_forward_backward_flag = FORWARD;
			}
		}

		// 세탁모드에서 드럼통을 앞뒤로 흔들어주기 위해서..
		if (dcmotor_forward_backward_flag == 0)
		{
			DCmotor_Forward_Rotate();
		}
		else
		{
			DCmotor_Backward_Rotate();
		}

		LEDbar_Flower_On();
		FND4digit_time_display(wash_remain_time);
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		mode_complete_alarm_stop_start_flag = START;
		wash_mode_start_stop_flag = STOP;

		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();

		// 완료가 되었다면 그냥 세탁 셋팅 화면을 다시 출력
		LCD_Command(CLEAR_DISPLAY);
		Wash_Mode_Setting_display();
	}
	/************************END 기본 동작 부분************************/

	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	// 세탁 모드 상태를 기억한 상태로 다음 모드로 이동
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status++;
		WashingMachine_curr_status %= 4;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Rinse_Mode_Setting_display();
	}

	// 세탁 횟수를 1회 증가시킨다(1회는 10초로 간주)
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		wash_remain_time += 10;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Wash_Mode_Setting_display();
	}

	// 세탁 횟수를 1회 감소시킨다.(1회는 10초로 간주)
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		wash_remain_time -= 10;
		if (wash_remain_time < 0) {wash_remain_time = 0;}
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Wash_Mode_Setting_display();
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
	{
		wash_mode_start_stop_flag = START;

		HAL_RTC_GetTime(&hrtc, &wash_complete_time, RTC_FORMAT_BCD);
		wash_complete_time.Hours += dec2bcd(wash_remain_time / 3600);
		wash_complete_time.Minutes += dec2bcd((wash_remain_time % 3600) / 60);
		wash_complete_time.Seconds += dec2bcd((wash_remain_time % 3600) % 60);

		if (bcd2dec(wash_complete_time.Seconds) >= 60)
		{
			wash_complete_time.Minutes += dec2bcd(1);
			wash_complete_time.Seconds -= dec2bcd(60);
		}
		if (bcd2dec(wash_complete_time.Minutes) >= 60)
		{
			wash_complete_time.Hours += dec2bcd(1);
			wash_complete_time.Minutes -= dec2bcd(60);
		}
		if (bcd2dec(wash_complete_time.Hours) >= 24)
		{
			// DATE를 1 올려야 하지만, 현재 해당 프로그램에서 date는 고려하고 있지 않음
			wash_complete_time.Hours -= dec2bcd(24);
		}

		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Wash_Mode_Running_display(wash_complete_time);
	}

	// 모터가 과열 되었거나, 버튼4를 눌렀을 때, 세탁 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Check_Motor_OverHeat() || Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status = IDLE_MODE;
		wash_mode_start_stop_flag = STOP;
		dcmotor_forward_backward_flag = 0;
		wash_remain_time = 0;
		wash_complete_time.Hours = dec2bcd(0);
		wash_complete_time.Minutes = dec2bcd(0);
		wash_complete_time.Seconds = dec2bcd(0);

		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Idle_Mode_Display(); // idle모드 화면으로 이동한다.
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

/*
 * desc: "세탁/헹굼/탈수" 중 "헹굼"에 관련된 동작을 실행하는 함수이다.
 */
static void Rinse_Mode_Laundry(void)
{
	static uint8_t rinse_mode_start_stop_flag = STOP;

	/************************BEGIN 기본 동작 부분************************/
	close_WashingMachine_Lid();
	Mode_Complete_Alarm();


	// 뚜껑이 열려있으면 동작을 중지하고 idle 화면으로 이동
	if (Check_Lid_Open())
	{
		mode_complete_alarm_stop_start_flag = STOP;
		WashingMachine_curr_status = 0;
		LCD_Command(CLEAR_DISPLAY);
		Idle_Mode_Display();
	}


	if (rinse_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();

	}
	else if (rinse_mode_start_stop_flag == START && rinse_remain_time > 0)
	{
		// 완료부저가 실행중이더라도, 다시 모드가 시작되면 당연히 부저는 꺼져야함
		mode_complete_alarm_stop_start_flag = STOP;

		// 남은시간은 자동으로 1초씩 감소
		if (TIM10_10ms_WM_rinse_timer > 100)
		{
			TIM10_10ms_WM_rinse_timer = 0;
			rinse_remain_time--;
			if (rinse_remain_time < 0) {rinse_remain_time = 0;}
		}

		DCmotor_Forward_Rotate();

		LEDbar_On_Up();
		FND4digit_time_display(rinse_remain_time);
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		mode_complete_alarm_stop_start_flag = START;
		rinse_mode_start_stop_flag = STOP;

		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();

		// 완료가 되었다면 그냥 헹굼 셋팅 화면을 다시 출력
		LCD_Command(CLEAR_DISPLAY);
		Rinse_Mode_Setting_display();
	}
	/************************END 기본 동작 부분************************/

	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status++;
		WashingMachine_curr_status %= 4;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Spin_Mode_Setting_display();
	}

	// 헹굼 횟수를 1회 증가시킨다(1회는 7초로 간주)
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		rinse_remain_time += 7;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Rinse_Mode_Setting_display();
	}

	// 헹굼 횟수를 1회 감소시킨다(1회는 7초로 간주)
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		rinse_remain_time -= 7;
		if (rinse_remain_time < 0) {rinse_remain_time = 0;}
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Rinse_Mode_Setting_display();
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
	{
		rinse_mode_start_stop_flag = START;

		HAL_RTC_GetTime(&hrtc, &rinse_complete_time, RTC_FORMAT_BCD);
		rinse_complete_time.Hours += dec2bcd(rinse_remain_time / 3600);
		rinse_complete_time.Minutes += dec2bcd((rinse_remain_time % 3600) / 60);
		rinse_complete_time.Seconds += dec2bcd((rinse_remain_time % 3600) % 60);

		if (bcd2dec(rinse_complete_time.Seconds) >= 60)
		{
			rinse_complete_time.Minutes += dec2bcd(1);
			rinse_complete_time.Seconds -= dec2bcd(60);
		}
		if (bcd2dec(rinse_complete_time.Minutes) >= 60)
		{
			rinse_complete_time.Hours += dec2bcd(1);
			rinse_complete_time.Minutes -= dec2bcd(60);
		}
		if (bcd2dec(rinse_complete_time.Hours) >= 24)
		{
			// DATE를 1 올려야 하지만, 현재 해당 프로그램에서 date는 고려하고 있지 않음
			rinse_complete_time.Hours -= dec2bcd(24);
		}

		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Rinse_Mode_Running_display(rinse_complete_time);
	}

	// 모터가 과열 되었거나, 버튼4를 눌렀을 때, 헹굼 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Check_Motor_OverHeat() || Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status = IDLE_MODE;
		rinse_mode_start_stop_flag = STOP;
		rinse_remain_time = 0;
		rinse_complete_time.Hours = dec2bcd(0);
		rinse_complete_time.Minutes = dec2bcd(0);
		rinse_complete_time.Seconds = dec2bcd(0);

		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Idle_Mode_Display(); // idle모드 화면으로 이동한다.
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

/*
 * desc: "세탁/헹굼/탈수" 중 "탈수"에 관련된 동작을 실행하는 함수이다.
 */
static void Spin_Mode_Laundry(void)
{
	static uint8_t spin_mode_start_stop_flag = STOP;
	static uint16_t dcmotor_duty_cycle = 30;


	/************************BEGIN 기본 동작 부분************************/
	close_WashingMachine_Lid();
	Mode_Complete_Alarm();

	// 뚜껑이 열려있으면 동작을 중지하고 idle 화면으로 이동
	if (Check_Lid_Open())
	{
		WashingMachine_curr_status = 0;
		LCD_Command(CLEAR_DISPLAY);
		Idle_Mode_Display();
	}


	if (spin_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();
	}
	else if (spin_mode_start_stop_flag == START && spin_remain_time > 0)
	{
		// 완료부저가 실행중이더라도, 다시 모드가 시작되면 당연히 부저는 꺼져야함
		mode_complete_alarm_stop_start_flag = STOP;

		// 남은시간은 자동으로 1초씩 감소
		if (TIM10_10ms_WM_spin_timer > 100)
		{
			TIM10_10ms_WM_spin_timer = 0;
			spin_remain_time--;
			if (spin_remain_time < 0) {spin_remain_time = 0;}

			dcmotor_duty_cycle += 3;
			if (dcmotor_duty_cycle > 100)
			{
				dcmotor_duty_cycle = 100;
			}

			DCmotor_Set_DutyCycle(dcmotor_duty_cycle);
		}

		DCmotor_Forward_Rotate();

		LEDbar_Keepon_Up();
		FND4digit_time_display(spin_remain_time);
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		mode_complete_alarm_stop_start_flag = START;
		spin_mode_start_stop_flag = STOP;
		dcmotor_duty_cycle = 30;

		DCmotor_Break();
		LEDbar_All_Off();
		FND4digit_off();

		// 완료가 되었다면 그냥 탈수 셋팅 화면을 다시 출력
		LCD_Command(CLEAR_DISPLAY);
		Spin_Mode_Setting_display();

//		Mode_Complete_Alarm();
	}
	/************************END 기본 동작 부분************************/

	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status++;
		WashingMachine_curr_status %= 4;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Idle_Mode_Display();
	}

	// 탈수 횟수를 1회 증가시킨다(1회는 7초로 간주)
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		spin_remain_time += 7;
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Spin_Mode_Setting_display();
	}

	// 헹굼 횟수를 1회 감소시킨다(1회는 7초로 간주)
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		spin_remain_time -= 7;
		if (spin_remain_time < 0) {spin_remain_time = 0;}
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Spin_Mode_Setting_display();
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
	{
		spin_mode_start_stop_flag = START;

		HAL_RTC_GetTime(&hrtc, &spin_complete_time, RTC_FORMAT_BCD);
		spin_complete_time.Hours += dec2bcd(spin_remain_time / 3600);
		spin_complete_time.Minutes += dec2bcd((spin_remain_time % 3600) / 60);
		spin_complete_time.Seconds += dec2bcd((spin_remain_time % 3600) % 60);

		if (bcd2dec(spin_complete_time.Seconds) >= 60)
		{
			spin_complete_time.Minutes += dec2bcd(1);
			spin_complete_time.Seconds -= dec2bcd(60);
		}
		if (bcd2dec(spin_complete_time.Minutes) >= 60)
		{
			spin_complete_time.Hours += dec2bcd(1);
			spin_complete_time.Minutes -= dec2bcd(60);
		}
		if (bcd2dec(spin_complete_time.Hours) >= 24)
		{
			// DATE를 1 올려야 하지만, 현재 해당 프로그램에서 date는 고려하고 있지 않음
			spin_complete_time.Hours -= dec2bcd(24);
		}

		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Spin_Mode_Running_display(spin_complete_time);
	}

	// 모터가 과열 되었거나, 버튼4를 눌렀을 때, 탈수 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		mode_complete_alarm_stop_start_flag = STOP; // 완료 부저가 진행중일 경우 아예 꺼버리는게 논리적으로 맞음

		WashingMachine_curr_status = IDLE_MODE;
		spin_mode_start_stop_flag = STOP;
		dcmotor_duty_cycle = 30;
		spin_remain_time = 0;
		spin_complete_time.Hours = dec2bcd(0);
		spin_complete_time.Minutes = dec2bcd(0);
		spin_complete_time.Seconds = dec2bcd(0);
		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
		Idle_Mode_Display();
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

///*
// * desc: "세탁/헹굼/탈수" 중 "세탁"에 관련된 동작을 실행하는 함수이다.
// *       세탁모드에서 드럼통 모터는 70%의 고정 duty cycle 출력으로 작동한다.
// */
//static void Auto_Mode_Laundry(void)
//{
//	static uint8_t auto_mode_start_stop_flag = STOP;
//	static uint8_t dcmotor_forward_backward_flag = FORWARD;
//	static uint32_t wash_remain_time = 0;
//	static RTC_TimeTypeDef wash_complete_time = {0};
//
//	/************************BEGIN 기본 동작 부분************************/
//	WashingMachine_3Mode_Display(wash_mode_start_stop_flag, wash_remain_time, wash_complete_time);
//	close_WashingMachine_Lid();
//
//	// 뚜껑이 열려있으면 동작을 중지하고 idle 화면으로 이동
//	if (Check_Lid_Open())
//	{
//		WashingMachine_curr_status = 0;
//		LCD_Command(CLEAR_DISPLAY);
//	}
//
//
//	if (wash_mode_start_stop_flag == STOP)
//	{
//		DCmotor_Break();
//	}
//	else if (wash_mode_start_stop_flag == START && wash_remain_time > 0)
//	{
//		// 남은시간은 자동으로 1초씩 감소
//		if (TIM10_10ms_WM_wash_timer > 100)
//		{
//			TIM10_10ms_WM_wash_timer = 0;
//			wash_remain_time--;
//			if (wash_remain_time < 0) {wash_remain_time = 0;}
//
//			if (dcmotor_forward_backward_flag == FORWARD)
//			{
//				dcmotor_forward_backward_flag = BACKWARD;
//			}
//			else
//			{
//				dcmotor_forward_backward_flag = FORWARD;
//			}
//		}
//
//		// 세탁모드에서 드럼통을 앞뒤로 흔들어주기 위해서..
//		if (dcmotor_forward_backward_flag == 0)
//		{
//			DCmotor_Forward_Rotate();
//		}
//		else
//		{
//			DCmotor_Backward_Rotate();
//		}
//
//		LEDbar_Flower_On();
//		FND4digit_time_display(wash_remain_time);
//	}
//	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
//	{
//		DCmotor_Break();
//		LEDbar_All_Off();
//		FND4digit_off();
//		wash_mode_start_stop_flag = STOP;
//		Mode_Complete_Alarm();
//	}
//	/************************END 기본 동작 부분************************/
//
//	/************************BEGIN 버튼 입력 인터럽트 부분************************/
//	// 세탁 모드 상태를 기억한 상태로 다음 모드로 이동
//	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
//	{
//		WashingMachine_curr_status++;
//		WashingMachine_curr_status %= 4;
//		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
//	}
//
//	// 세탁에 소요할 시간을 10초 증가시킨다.
//	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
//	{
//		wash_remain_time += 60;
//		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
//	}
//
//	// 세탁에 소요할 시간을 1초 증가시킨다.
//	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
//	{
//		wash_remain_time += 1;
//		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
//	}
//
//	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
//	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
//	{
//		wash_mode_start_stop_flag = START;
//
//		HAL_RTC_GetTime(&hrtc, &wash_complete_time, RTC_FORMAT_BCD);
//		wash_complete_time.Hours += dec2bcd(wash_remain_time / 3600);
//		wash_complete_time.Minutes += dec2bcd((wash_remain_time % 3600) / 60);
//		wash_complete_time.Seconds += dec2bcd((wash_remain_time % 3600) % 60);
//
//		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
//
////		Mode_Start_Alarm();
//	}
//
//	// 세탁 모드를 즉시 중지하고 세탁모드를 초기화해준다.
//	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
//	{
//		WashingMachine_curr_status = IDLE_MODE;
//		wash_mode_start_stop_flag = STOP;
//		dcmotor_forward_backward_flag = 0;
//		wash_remain_time = 0;
//		wash_complete_time.Hours = dec2bcd(0);
//		wash_complete_time.Minutes = dec2bcd(0);
//		wash_complete_time.Seconds = dec2bcd(0);
//
//		LCD_Command(CLEAR_DISPLAY); // 버튼을 누르는 순간마다 clear display해줘서 쓰레기값 방지
//	}
//	/************************END 버튼 입력 인터럽트 부분************************/
//}


/*
 * desc: 세탁기의 뚜껑이 열린상태인지 닫힌 상태인지 파악한다. 뚜껑의 개폐 상태는 1초에 한번씩 체크한다.
 *       초음파센서와 뚜껑의 거리가 10cm 이하이면 닫힌 것으로 간주하고, 10cm 초과이면 열린 것으로 간주한다.
 * return: True(뚜껑이 열려있음) False(뚜껑이 닫혀있음)
 */
static bool Check_Lid_Open(void)
{
	int32_t distance;

#if 0 // 코드 디버깅 편리성을 위해 잠시 비활성화

	if (TIM10_10ms_counter_ultrasonic >= LID_OPEN_CHECK_TERM)
	{
		TIM10_10ms_counter_ultrasonic = 0;
		Ultrasonic_Trigger();

		if (one_cycle_capture_finish_flag)
		{
			one_cycle_capture_finish_flag = 0;
			distance = ultrasonic_distance;
			distance = distance * 0.034 / 2;

			if (distance > LID_SAFTY_LIMIT)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

#else

	return false;

#endif
}

/*
 * desc: 세탁기의 모터가 과열 상태인지 파악한다. 세탁기의 과열 상태는 10초에 한번씩 파악한다.
 * 		 DHT11 센서가 리턴한 온도 값이 70도(섭씨) 초과이면 과열된 것으로 간주하고, 70도(섭씨) 미만이면 정상인 것으로 간주한다.
 * return: True(모터가 과열되어 있음) False(모터가 정상온도임)
 */
static bool Check_Motor_OverHeat(void)
{
	uint8_t i_RH, d_RH, i_Tmp, d_Tmp;

	if (TIM10_10ms_counter_DHT11 >= MOTOR_HEAT_CHECK_TERM)
	{
		TIM10_10ms_counter_DHT11 = 0;

		// 아래 세줄은 DHT11과 MCU의 Hand-shaking과정이다.
		DHT11_Trriger();
		DHT11_DataLine_Input();
		DHT11_Dumi_Read();

		// 여기부터 DHT11가 수집한 데이터를 읽어오는 것이다.
		i_RH = DHT11_rx_Data();
		d_RH = DHT11_rx_Data();
		i_Tmp = DHT11_rx_Data();
		d_Tmp = DHT11_rx_Data();

		DHT11_DataLine_Output();
		HAL_GPIO_WritePin(DHT11_PORT, DHT11_DATA_RIN, GPIO_PIN_SET);

		if (i_Tmp > MOTOTR_HEAT_SAFTY_LIMIT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

/*
 * desc: 세탁기가 IDLE 모드 일때의 화면을 I2C_LCD에 출력한다.
 */
static void Idle_Mode_Display(void)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BCD);

	sprintf(lcd_buff_1, "select [MODE]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	if (old_time.Seconds != current_time.Seconds)
	{
		sprintf(lcd_buff_2, "Now>> %02d:%02d:%02d", bcd2dec(current_time.Hours), bcd2dec(current_time.Minutes), bcd2dec(current_time.Seconds));
		Move_Cursor(1, 0);
		LCD_String(lcd_buff_2);
	}
	old_time.Seconds = current_time.Seconds;
}

/*
 * desc: 세탁기가 세탁모드 일때, 세탁을 몇회 시행할 예정인지 대한 정보를 I2C_LCD에 출력한다.
 */
static void Wash_Mode_Setting_display(void)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Wash]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Repeat [%d]times", wash_remain_time / 10);
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: 세탁기가 세탁모드 일때, 예정된 세탁을 모두 시행할 경우 예상 완료시간이 언제인지에 대한 정보를 I2C_LCD에 출력한다.
 * param1: 예상 완료시간에 대한 정보를 담고 있는 RTC_Time 구조체
 */
static void Wash_Mode_Running_display(RTC_TimeTypeDef complete_time)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Wash]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Cplt>> %02d:%02d:%02d", bcd2dec(complete_time.Hours), bcd2dec(complete_time.Minutes), bcd2dec(complete_time.Seconds));
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: 세탁기가 헹굼모드 일때, 헹굼을 몇회 시행할 예정인지 대한 정보를 I2C_LCD에 출력한다.
 */
static void Rinse_Mode_Setting_display(void)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Rinse]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Repeat [%d]times", rinse_remain_time / 10);
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: 세탁기가 헹굼모드 일때, 예정된 헹굼을 모두 시행할 경우 예상 완료시간이 언제인지에 대한 정보를 I2C_LCD에 출력한다.
 * param1: 예상 완료시간에 대한 정보를 담고 있는 RTC_Time 구조체
 */
static void Rinse_Mode_Running_display(RTC_TimeTypeDef complete_time)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Rinse]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Cplt>> %02d:%02d:%02d", bcd2dec(complete_time.Hours), bcd2dec(complete_time.Minutes), bcd2dec(complete_time.Seconds));
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: 세탁기가 탈수모드 일때, 탈수를 몇회 시행할 예정인지 대한 정보를 I2C_LCD에 출력한다.
 */
static void Spin_Mode_Setting_display(void)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Spin]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Repeat [%d]times", spin_remain_time / 10);
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: 세탁기가 탈수모드 일때, 예정된 탈수를 모두 시행할 경우 예상 완료시간이 언제인지에 대한 정보를 I2C_LCD에 출력한다.
 * param1: 예상 완료시간에 대한 정보를 담고 있는 RTC_Time 구조체
 */
static void Spin_Mode_Running_display(RTC_TimeTypeDef complete_time)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "  Manual [Spin]");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "Cplt>> %02d:%02d:%02d", bcd2dec(complete_time.Hours), bcd2dec(complete_time.Minutes), bcd2dec(complete_time.Seconds));
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

