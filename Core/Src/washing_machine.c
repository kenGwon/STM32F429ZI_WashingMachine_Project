#include "washing_machine.h"

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern uint8_t rx_data;
extern uint8_t bt_rx_data;

extern volatile uint32_t TIM10_10ms_WM_wash_timer;
extern volatile uint32_t TIM10_10ms_WM_rinse_timer;
extern volatile uint32_t TIM10_10ms_WM_spin_timer;

static uint8_t WashingMachine_curr_status = IDLE_MODE;
static uint8_t wash_mode_start_stop_flag = STOP;
static uint8_t rinse_mode_start_stop_flag = STOP;
static uint8_t spin_mode_start_stop_flag = STOP;

static void Idle_Mode_Laundry(void);
static void Wash_Mode_Laundry(void);
static void Rinse_Mode_Laundry(void);
static void Spin_Mode_Laundry(void);
static bool Check_Lid_Open(void);
static void WashingMachine_LCD_Display(void);

/*
 * desc: 세탁기 프로그램 구동에 필요한 H/W를 켠다.
 */
void WashingMachine_Init(void)
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // 세탁기 뚜껑 개폐용 SERVO motor 구동하기 위해 필요
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1); // 세탁기 뚜껑 개폐 상태 확인하는 초음파센서 활용하기 위해 필요
	HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1); // 드럼통 동작용 DC motor 구동하기 위해 필요
//	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4); // 세탁, 헹굼, 탈수 동작완료 안내 부저 작동을 위해 필요
	HAL_TIM_Base_Start_IT(&htim10); // 다수의 10ms 타이머 활옹하기 위해 필요
	HAL_TIM_Base_Start_IT(&htim11); // DHT11센서에서 delay_us() 함수가 올바르게 작동해야 하므로 필요
	DHT11_Init(); // 탈수 완료 여부 확인하는 온습도 센서 활용하기 위해 필요
	I2C_LCD_Init(); // 세탁기 상태 보여주는 LCD 활용하기 위해 필요

	HAL_UART_Receive_IT(&huart3, &rx_data, 1); // activate interrupt from RX huart3
	HAL_UART_Receive_IT(&huart6, &bt_rx_data, 1); // activate interrupt from RX huart6
}

/*
 * desc: 세탁기 프로그램 구동에 사용했던 H/W를 끈다.
 */
void WashingMachine_Terminate(void)
{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
//	HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);
	HAL_TIM_Base_Stop_IT(&htim10);
	HAL_TIM_Base_Stop_IT(&htim11);
//	DHT11_Terminate();
//	I2C_LCD_Terminate();
}

/*
 * desc: 세탁기 프로그램의 메인 동작을 결정하는 함수이다.
 */
void WashingMachine_Processing(void)
{
	// 현재 세탁기 상태에 맞는 디스플레이 출력
	WashingMachine_LCD_Display();

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
	WashingMachine_LCD_Display();
	DCmotor_Break();

	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = WASH_MODE;
	}
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = RINSE_MODE;
	}
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = SPIN_MODE;
	}


}




/*
 * desc: "세탁/헹굼/탈수" 중 "세탁"에 관련된 동작을 실행하는 함수이다.
 *       세탁모드에서 드럼통 모터는 70%의 고정 duty cycle 출력으로 작동한다.
 */
static void Wash_Mode_Laundry(void)
{
	static uint32_t wash_remain_time = 0;
	static uint8_t dcmotor_forward_backward_flag = 0;

printf("wash_remain_time: %d\n", wash_remain_time);
printf("wash_mode_start_stop_flag: %d\n", wash_mode_start_stop_flag);

	close_WashingMachine_Lid();

	/************************BEGIN 기본 동작 부분************************/
	if (wash_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
	}
	else if (wash_mode_start_stop_flag == START && wash_remain_time > 0)
	{
		// 남은시간은 자동으로 1초씩 감소
		if (TIM10_10ms_WM_wash_timer > 100)
		{
			TIM10_10ms_WM_wash_timer = 0;
			wash_remain_time--;
			if (wash_remain_time < 0) {wash_remain_time = 0;}

			if (dcmotor_forward_backward_flag == 0)
			{
				dcmotor_forward_backward_flag = 1;
			}
			else
			{
				dcmotor_forward_backward_flag = 0;
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
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		DCmotor_Break();
		wash_mode_start_stop_flag = STOP;
	}
	/************************END 기본 동작 부분************************/

	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	// 세탁에 소요할 시간을 10초 증가시킨다.
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		wash_remain_time += 10;
	}

	// 세탁에 소요할 시간을 10초 감소시킨다.
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		wash_remain_time -= 10;
		if (wash_remain_time < 0) {wash_remain_time = 0;}
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		wash_mode_start_stop_flag = START;
	}

	// 세탁 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = IDLE_MODE;
		wash_mode_start_stop_flag = STOP;
		wash_remain_time = 0;
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

/*
 * desc: "세탁/헹굼/탈수" 중 "헹굼"에 관련된 동작을 실행하는 함수이다.
 */
static void Rinse_Mode_Laundry(void)
{
	static uint32_t rinse_remain_time = 0;

printf("rinse_remain_time: %d\n", rinse_remain_time);
printf("rinse_mode_start_stop_flag: %d\n", rinse_mode_start_stop_flag);

	close_WashingMachine_Lid();

	if (rinse_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
	}
	else if (rinse_mode_start_stop_flag == START && rinse_remain_time > 0)
	{
		// 남은시간은 자동으로 1초씩 감소
		if (TIM10_10ms_WM_rinse_timer > 100)
		{
			TIM10_10ms_WM_rinse_timer = 0;
			rinse_remain_time--;
			if (rinse_remain_time < 0) {rinse_remain_time = 0;}
		}

		DCmotor_Forward_Rotate();
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		DCmotor_Break();
		rinse_mode_start_stop_flag = STOP;
	}


	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	// 세탁에 소요할 시간을 10초 증가시킨다.
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		rinse_remain_time += 10;
	}

	// 세탁에 소요할 시간을 10초 감소시킨다.
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		rinse_remain_time -= 10;
		if (rinse_remain_time < 0) {rinse_remain_time = 0;}
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		rinse_mode_start_stop_flag = START;
	}

	// 세탁 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = IDLE_MODE;
		rinse_mode_start_stop_flag = STOP;
		rinse_remain_time = 0;
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

/*
 * desc: "세탁/헹굼/탈수" 중 "탈수"에 관련된 동작을 실행하는 함수이다.
 */
static void Spin_Mode_Laundry(void)
{
	static uint32_t spin_remain_time = 0;
	static uint16_t dcmotor_duty_cycle = 30;

printf("spin_remain_time: %d\n", spin_remain_time);
printf("spin_mode_start_stop_flag: %d\n", spin_mode_start_stop_flag);
printf("dcmotor_duty_cycle: %d\n", dcmotor_duty_cycle);

	close_WashingMachine_Lid();

	if (spin_mode_start_stop_flag == STOP)
	{
		DCmotor_Break();
	}
	else if (spin_mode_start_stop_flag == START && spin_remain_time > 0)
	{
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
	}
	else // 플래그는 start이지만 남은시간이 0이하가 된 경우 여기로 빠짐
	{
		dcmotor_duty_cycle = 30;
		DCmotor_Break();
		spin_mode_start_stop_flag = STOP;
	}


	/************************BEGIN 버튼 입력 인터럽트 부분************************/
	// 세탁에 소요할 시간을 10초 증가시킨다.
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		spin_remain_time += 10;
	}

	// 세탁에 소요할 시간을 10초 감소시킨다.
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		spin_remain_time -= 10;
		if (spin_remain_time < 0) {spin_remain_time = 0;}
	}

	// 동작 시간을 설정 완료했다면, 세탁기 구동을 시작한다.
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		spin_mode_start_stop_flag = START;
	}

	// 세탁 모드를 즉시 중지하고 세탁모드를 초기화해준다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		WashingMachine_curr_status = IDLE_MODE;
		spin_mode_start_stop_flag = STOP;
		spin_remain_time = 0;
		dcmotor_duty_cycle = 30;
	}
	/************************END 버튼 입력 인터럽트 부분************************/
}

/*
 * desc: 세탁기의 뚜껑이 열린상태인지 닫힌 상태인지 파악한다.
 *       초음파센서와 뚜껑의 거리가 5cm 이하이면 닫힌 것으로 간주하고, 5cm 초과이면 열린 것으로 간주한다.
 * return: True(뚜껑이 열려있음) False(뚜껑이 닫혀있음)
 */
static bool Check_Lid_Open(void)
{
	// 세탁기 뚜껑 개폐 동작은 서브모터를 통해 형상화 할 수 있다 .
}

/*
 * desc: "세탁/헹굼/탈수" 중 세탁기의 현재 모드에 맞는 LCD화면을 출력한다.
 */
static void WashingMachine_LCD_Display(void)
{
	// i2c lcd 화면 클리어 하는법: i2c_lcd.c파일에 있는 lcd_command(CLEAR_DISPLAY); 함수를 사용하면 된다.

	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	switch (WashingMachine_curr_status)
	{
		case IDLE_MODE:
			sprintf(lcd_buff_1, "[ select MODE  ]");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);

			sprintf(lcd_buff_2, "WASH/RINSE/SPIN");
			Move_Cursor(1, 0);
			LCD_String(lcd_buff_2);
			break;

		case WASH_MODE:
			sprintf(lcd_buff_1, "[  WASH_MODE   ]");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);

			sprintf(lcd_buff_2, "      ...       ");
			Move_Cursor(1, 0);
			LCD_String(lcd_buff_2);
			break;

		case RINSE_MODE:
			sprintf(lcd_buff_1, "[  RINSE_MODE  ]");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);

			sprintf(lcd_buff_2, "      ...       ");
			Move_Cursor(1, 0);
			LCD_String(lcd_buff_2);
			break;

		case SPIN_MODE:
			sprintf(lcd_buff_1, "[  SPIN_MODE   ]");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);

			sprintf(lcd_buff_2, "      ...       ");
			Move_Cursor(1, 0);
			LCD_String(lcd_buff_2);
			break;

		default:
			break;
	}
}

