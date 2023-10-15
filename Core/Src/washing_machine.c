#include "washing_machine.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;


/*
 * desc: 세탁기 프로그램 구동에 필요한 H/W를 켠다.
 */
void WashingMachine_Init(void)
{
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1); // 세탁기 뚜껑 개폐 상태 확인하는 초음파센서 활용하기 위해 필요
	HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1); // 드럼통 모터 구동하기 위해 필요
	HAL_TIM_Base_Start_IT(&htim10); // 다수의 10ms 타이머 활옹하기 위해 필요
	HAL_TIM_Base_Start_IT(&htim11); // DHT11센서에서 delay_us() 함수가 올바르게 작동해야 하므로 필요
	DHT11_Init(); // 탈수 완료 여부 확인하는 온습도 센서 활용하기 위해 필요
	I2C_LCD_Init(); // 세탁기 상태 보여주는 LCD 활용하기 위해 필요
}

/*
 * desc: 세탁기 프로그램 구동에 사용했던 H/W를 끈다.
 */
void WashingMachine_Terminate(void)
{
	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
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

}


/*
 * desc: "세탁/헹굼/탈수" 중 "세탁"에 관련된 동작을 실행하는 함수이다.
 */
void Wash_Laundry(void)
{

}

/*
 * desc: "세탁/헹굼/탈수" 중 "헹굼"에 관련된 동작을 실행하는 함수이다.
 */
void Rinse_Laundry(void)
{

}

/*
 * desc: "세탁/헹굼/탈수" 중 "탈수"에 관련된 동작을 실행하는 함수이다.
 */
void Spin_Laundry(void)
{

}

/*
 * desc: 세탁기의 뚜껑이 열린상태인지 닫힌 상태인지 파악한다.
 *       초음파센서와 뚜껑의 거리가 5cm 이하이면 닫힌 것으로 간주하고, 5cm 초과이면 열린 것으로 간주한다.
 * return: True(뚜껑이 열려있음) False(뚜껑이 닫혀있음)
 */
bool Check_Lid_Open(void)
{

}
