#include "fan_machine.h"

extern TIM_HandleTypeDef htim4;
extern volatile uint32_t TIM10_10ms_fan_auto_mode_remain_time_decrease;
extern volatile uint32_t TIM10_10ms_fan_rotate_direction_led;
extern volatile uint32_t TIM10_10ms_fan_lcd_display_clear;

uint8_t dcmotor_start_stop_flag = 0; // 0은 dcmotor stop상태, 1은 dcmotor start상태 [플래그 변수]
uint8_t dcmotor_forward_backward_flag = 0; // 0은 정방향, 1은 역방향 [플래그 변수]
uint8_t dcmotor_manual_auto_flag = 0; // 0은 manual mode, 1은 auto mode [플래그 변수]

uint16_t ChannelControlRegister_UP_Value = 0; // PWM UP control // 16비트 타이머니까 uint16_t로 선언함
uint16_t ChannelControlRegister_DOWN_Value = 0; // PWM DOWN control
int32_t auto_mode_remain_time = 30; // auto mode 남은시간

/*
 * desc: 현재 모드가 매뉴얼 모드면 매뉴얼 모드 컨트롤 함수를 실행하고, 현재 모드가 오토 모드면 오토 모드 컨트롤 함수를 실행한다.
 */
void Fan_Processing(void)
{
	if (dcmotor_manual_auto_flag == 0)
	{
		Fan_Manual_Control();
	}
	else
	{
		Fan_Auto_Control();
	}

	// dcmotor의 현재 상태를 출력해주는 I2C LCD 디스플레이에 쓰레기 값이 출력되는 것을 막기 위해, 1초마다 LCD의 모든 칸을 공백으로 초기화 해주는 함수를 실행한다.
	// (임시방편임. 원래는 i2c_lcd.c단에서 수정하는게 맞을듯?)
	if (TIM10_10ms_fan_lcd_display_clear >= 100)
	{
		TIM10_10ms_fan_lcd_display_clear = 0;
		Clear_Fan_LCD_Display();
	}
}

/*
 * desc: 매뉴얼 모드 상태에서 dcmotor의 동작방식을 결정한다.
 */
void Fan_Manual_Control(void)
{
	Fan_LCD_Display();

	// dcmotor가 stop 상태라면 그에 맞는 값을 모터 드라이버에 입력해주고,
	// dcmotor가 start 상태라면 그에 맞는 값을 모터 드라이버에 입력해준다.
	if (!dcmotor_start_stop_flag)
	{
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0);

		// 동작 상태가 아니므로 "선풍기의 세기를 알리는 LED Bar"와 "정회전 역회전을 알리는 led"를 모두 끈다.
		HAL_GPIO_WritePin(GPIOD, 0xff, 0);
		HAL_GPIO_WritePin(FAN_ROTATE_DERECTION_LED_GPIO_Port, FAN_ROTATE_DERECTION_LED_Pin, 0);
	}
	else
	{
		// dcmotor가 start 상태이면서 정방향 회전중이라면, 그에 맞는 값을 모터 드라이버에 넣어주고
		// dcmotor가 start 상태이면서 역방향 회전중이라면, 그에 맞는 값을 모터 드라이버에 넣어준다.
		if (!dcmotor_forward_backward_flag)
		{
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
		}

		// 동작 상태가 맞으므로 "선풍기의 세기를 알리는 LED Bar"와 "정회전 역회전을 알리는 led"를 모두 켠다.
		Fan_LEDbar_Control(Get_Current_ChannelControlRegister_Value());
		Fan_Rotate_Direction_LED_Control(dcmotor_forward_backward_flag);
	}

	// BUTTON0: dcmotor의 start/stop 상태를 토글한다.
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // 작동 로직과 관계 없는 함수 실행 확인용 테스트 코드.

		// dcmotor의 start/stop 상태를 의미하는 플래그 변수를 활용하여, 버튼을 눌렀을 때 상태가 반전되도록 한다.
		if (!dcmotor_start_stop_flag)
		{
			dcmotor_start_stop_flag = 1;
		}
		else
		{
			dcmotor_start_stop_flag = 0;
		}
	}

	// BUTTON1: 듀티 사이클을 상승시켜 선풍기의 회전 속도를 가속한다.
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // 작동 로직과 관계 없는 함수 실행 확인용 테스트 코드.

		ChannelControlRegister_UP_Value = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
		ChannelControlRegister_UP_Value += 10;

		if (ChannelControlRegister_UP_Value > 100) // ChannelControlRegister_UP_Value의 쓰레시 홀드(최대치 상한): 듀디사이클 최대 상한치는 100%(풀 스피드)
		{
			ChannelControlRegister_UP_Value = 100;
		}

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ChannelControlRegister_UP_Value);

		Fan_LEDbar_Control(Get_Current_ChannelControlRegister_Value());
	}

	// BUTTON2: 듀티 사이클을 하락시켜 선풍기의 회전 속도를 감속한다.
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED1

		ChannelControlRegister_DOWN_Value = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
		ChannelControlRegister_DOWN_Value -= 10;

		if (ChannelControlRegister_DOWN_Value < 60) // ChannelControlRegister_DOWN_Value의 쓰레시 홀드(최대치 하한): 듀디사이클 펄스 최대 하한치는 60%
		{
			ChannelControlRegister_DOWN_Value = 60;
		}

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ChannelControlRegister_DOWN_Value);

		Fan_LEDbar_Control(Get_Current_ChannelControlRegister_Value());
	}

	// BUTTON3: 모터 드라이버에 입력신호를 바꿔서 dcmotor의 회전 방향을 정회전/역회전 두가지 상태 중 하나로 조작한다.
	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
	{

		// dcmotor의 정회전/역회전 상태를 의미하는 플래그 변수를 활용하여
		// 플래그 변수가 정회전 상태를 가리키게 되면, 모터 드라이버에 해당하는 값을 입력해주고, 정회전 상태에 해당하는 상태로 led깜빡이가 깜빡거릴 수 있도록 플래그변수를 매개변수로 담아 관련 함수를 한번 호출해준다,
		// 플래그 변수가 역회전 상태를 가리키게 되면, 모터 드라이버에 해당하는 값을 입력해주고, 역회전 상태에 해당하는 상태로 led깜빡이가 깜빡거릴 수 있도록 플래그변수를 매개변수로 담아 관련 함수를 한번 호출해준다
		if (!dcmotor_forward_backward_flag)
		{
			dcmotor_forward_backward_flag = 1;
			Fan_Rotate_Direction_LED_Control(dcmotor_forward_backward_flag);
		}
		else
		{
			dcmotor_forward_backward_flag = 0;
			Fan_Rotate_Direction_LED_Control(dcmotor_forward_backward_flag);
		}

	}

	// BUTTON4: dcmotor의 매뉴얼/오토 상태를 의미하는 플래그 변수를 활용하여, dcmotor의 상태를 오토모드로 변경한다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		dcmotor_manual_auto_flag = 1;
	}
}

/*
 * desc: 오토 모드 상태에서 dcmotor의 동작방식을 결정한다.
 */
void Fan_Auto_Control(void)
{
	Fan_LCD_Display();

	// 오토 모드에서 남은 시간은 기본 30초로 셋팅되어 있으며, 1초마다 1씩 감소한다.
	if (TIM10_10ms_fan_auto_mode_remain_time_decrease >= 100)
	{
		TIM10_10ms_fan_auto_mode_remain_time_decrease = 0;
		auto_mode_remain_time--;

		// 오토모드에서 시간이 남아있으면, 선풍기는 정방향으로만 회전하도록 한다.
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0);

		if (auto_mode_remain_time < 0)
		{
			// 오토모드에서 남은 시간이 0초가 되면, 모터 드라이버에 대한 입력 값을 1 1로 주어서 break상태로 만든다.
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);

			auto_mode_remain_time = 0; // 남은시간이 마이너스로 떨어지는거 방지
		}
	}

	// BUTTON0: 남은시간이 10초 증가한다.
	if (Get_Button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		auto_mode_remain_time += 10;
	}

	// BUTTON1: 남은시간이 10초 감소한다.
	if (Get_Button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
	{
		auto_mode_remain_time -= 10;
		if (auto_mode_remain_time < 0) auto_mode_remain_time = 0; // 남은시간이 마이너스로 떨어지는거 방지
	}

	// BUTTON2: 듀티 사이클을 상승시켜 선풍기의 회전 속도를 가속한다.
	if (Get_Button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // 작동 로직과 관계 없는 함수 실행 확인용 테스트 코드.

		ChannelControlRegister_UP_Value = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
		ChannelControlRegister_UP_Value += 10;

		if (ChannelControlRegister_UP_Value > 100) // ChannelControlRegister_UP_Value의 쓰레시 홀드(최대 상한치): 듀디사이클 최대 상한치는 100%(풀스피트)
		{
			ChannelControlRegister_UP_Value = 99;
		}

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ChannelControlRegister_UP_Value);

		Fan_LEDbar_Control(Get_Current_ChannelControlRegister_Value());
	}

	// BUTTON2: 듀티 사이클을 하락시켜 선풍기의 회전 속도를 감속한다.
	if (Get_Button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED1

		ChannelControlRegister_DOWN_Value = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
		ChannelControlRegister_DOWN_Value -= 10;

		if (ChannelControlRegister_DOWN_Value < 60) // ChannelControlRegister_DOWN_Value의 쓰레시 홀드(최대 하한치): 듀디사이클 펄스 최대 하한치는 60%
		{
			ChannelControlRegister_DOWN_Value = 60;
		}

		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ChannelControlRegister_DOWN_Value);

		Fan_LEDbar_Control(Get_Current_ChannelControlRegister_Value());
	}

	// BUTTON4: dcmotor의 매뉴얼/오토 상태를 의미하는 플래그 변수를 활용하여, dcmotor의 상태를 매뉴얼 모드로 변경한다.
	if (Get_Button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
	{
		dcmotor_manual_auto_flag = 0;
	}
}

/*
 * desc: dcmotor의 현재 상태를 출력해주는 I2C LCD 디스플레이에 쓰레기 값이 출력되는 것을 막기 위해 LCD의 모든 칸을 공백으로 초기화 해준다.(임시방편임. 원래는 i2c_lcd.c단에서 수정하는게 맞을듯?)
 */
void Clear_Fan_LCD_Display(void)
{
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	sprintf(lcd_buff_1, "                    ");
	Move_Cursor(0, 0);
	LCD_String(lcd_buff_1);

	sprintf(lcd_buff_2, "                    ");
	Move_Cursor(1, 0);
	LCD_String(lcd_buff_2);
}

/*
 * desc: dcmotor의 현재 상태에 맞는 정보를 I2C_LCD display에 출력한다.
 */
void Fan_LCD_Display(void)
{
	int manual_mode_curr_duty_cycle;
	int auto_mode_remain_time_lv;
	uint8_t lcd_buff_1[20], lcd_buff_2[20];

	if (!dcmotor_manual_auto_flag) // 매뉴얼 모드가 실행 중인 경우
	{
		manual_mode_curr_duty_cycle = Get_Current_ChannelControlRegister_Value();

		if (!dcmotor_start_stop_flag)
		{
			sprintf(lcd_buff_1, "Manual Mode stop!");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);
		}
		else
		{
			sprintf(lcd_buff_1, "Manual Mode run!");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);
		}


		sprintf(lcd_buff_2, "duty cycle: %d", manual_mode_curr_duty_cycle);
		Move_Cursor(1, 0);
		LCD_String(lcd_buff_2);
	}
	else // 오토 모드가 실행 중인 경우
	{
		auto_mode_remain_time_lv = auto_mode_remain_time; // 오토모드 남은시간 전역변수를 지역변수(lv: local variable)로 값 대피

		if (auto_mode_remain_time_lv > 0)
		{
			sprintf(lcd_buff_1, "Auto Mode run!");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);
		}
		else
		{
			sprintf(lcd_buff_1, "Auto Mode stop!");
			Move_Cursor(0, 0);
			LCD_String(lcd_buff_1);
		}

		sprintf(lcd_buff_2, "remain_time: %ds", auto_mode_remain_time_lv);
		Move_Cursor(1,0);
		LCD_String(lcd_buff_2);
	}
}

/*
 * desc: dcmotor의 정방향/역방향 상태에 따라, led의 점멸 간격이 달라지도록 한다.
 *       새로 부착한 led는 PE13번 포트에 GPIO_OUTPUT으로 연결하였으며, 유저 레이블을 FAN_ROTATE_DERECTION_LED로 명명했다.
 */
void Fan_Rotate_Direction_LED_Control(uint8_t forward_backwrd_dcmotor)
{
	if (!forward_backwrd_dcmotor) // 역방향 회전 중이라면
	{
		if (TIM10_10ms_fan_rotate_direction_led >= 5) // 50ms마다 점멸
		{
			TIM10_10ms_fan_rotate_direction_led = 0;
			HAL_GPIO_TogglePin(FAN_ROTATE_DERECTION_LED_GPIO_Port, FAN_ROTATE_DERECTION_LED_Pin);
		}
	}
	else // 정회전 중이라면
	{
		if (TIM10_10ms_fan_rotate_direction_led >= 10) // 100ms마다 점멸
		{
			TIM10_10ms_fan_rotate_direction_led = 0;
			HAL_GPIO_TogglePin(FAN_ROTATE_DERECTION_LED_GPIO_Port, FAN_ROTATE_DERECTION_LED_Pin);
		}
	}
}

/*
 * desc: dcmotor에 인가 되고 있는 듀티 사이클의 크기에 따라, ledbar에 들어오는 불빛의 갯수를 결정한다.
 *       이 코드의 경우 dcmotor의 최저 듀티 사이클인 60% 상태에서도 최소 2칸의 불빛이 들어오도록 했으며,
 *       dcmotor의 최대 듀티 사이클인 100% 상태에서도 최대 7칸 까지만 불빛이 들어오도록 했다.
 */
void Fan_LEDbar_Control(uint16_t Current_ChannelControlRegister_Value)
{
	int ledbar_level;

	ledbar_level = (Current_ChannelControlRegister_Value - 44) / 8;

	HAL_GPIO_WritePin(GPIOD, 0xff, 0);

	for (int i = 0; i < ledbar_level; i++)
	{
		HAL_GPIO_WritePin(GPIOD, 0x00|(0x80 >> i), 1);
	}
}

/*
 * desc: __HAL_TIM_GET_COMPARE(__HANDLE__, __CHANNEL__) 함수가 리턴하는 값의 자료형은 16비트일 수도 있고 32비트 일수도 있다.
 *       dcmotor에서 쓰고 있는 TIM4 타이머는 16비트 타이머이기 때문에, Channel Control Register가 16비트라는 뜻이고, 위 함수는 16비트 값을 리턴한다.
 *       (그냥 위에 함수 그대로 써도 되는데, 함수이름도 안와닿고 16비트 값을 받고 있다는 것을 명시하고 싶어서 그냥 만들어봤음)
 */
uint16_t Get_Current_ChannelControlRegister_Value(void)
{
	return __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
}
