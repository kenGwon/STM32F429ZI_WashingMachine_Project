#include "ledbar.h"

extern volatile uint32_t TIM10_10ms_counter_ledbar;

/*
 * desc: ledbar의 동작상태를 테스트 하기 위한 테스트함수
 */
void LEDbar_Test(void)
{
	while (1)
	{
		LEDbar_On_Up();
		LEDbar_On_Down();
		LEDbar_Keepon_Up();
		LEDbar_Keepon_Down();
		LEDbar_Flower_On();
		LEDbar_Flower_Off();
	}
}

/*
 * desc: ledbar가 모두 켜진다.
 */
void LEDbar_All_On(void)
{
	HAL_GPIO_WritePin(GPIOD, 0xff, 1);
}

/*
 * desc: ledbar가 모두 꺼진다.
 */
void LEDbar_All_Off(void)
{
	HAL_GPIO_WritePin(GPIOD, 0xff, 0);
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, 아래서부터 위로 한칸씩 올라가면서 1개의 led가 켜졌다가 꺼진다.
 */
void LEDbar_On_Up(void)
{
	int i = 0;

	LEDbar_All_Off();
	TIM10_10ms_counter_ledbar = 0;

	while (i < 8)
	{
		if (TIM10_10ms_counter_ledbar >= 20)
		{
			TIM10_10ms_counter_ledbar = 0;
			LEDbar_All_Off();
			HAL_GPIO_WritePin(GPIOD, 0x01 << i++, 1);
		}
	}
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, 아래서부터 위로 한칸씩 올라가면서 1개의 led가 켜졌다가 꺼진다.
 */
void LEDbar_On_Down(void)
{
	int i = 0;

	LEDbar_All_Off();
	TIM10_10ms_counter_ledbar = 0;

	while (i < 8)
	{
		if (TIM10_10ms_counter_ledbar >= 20)
		{
			TIM10_10ms_counter_ledbar = 0;
			LEDbar_All_Off();
			HAL_GPIO_WritePin(GPIOD, 0x80 >> i++, 1);
		}
	}
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, 아래서부터 위로 한칸씩 올라가면서 켜진 LED는 유지하면서 1개의 led가 추가로 켜진다.
 */
void LEDbar_Keepon_Up(void)
{
	int i = 0;

	LEDbar_All_Off();
	TIM10_10ms_counter_ledbar = 0;

	while (i < 8)
	{
		if (TIM10_10ms_counter_ledbar >= 20)
		{
			TIM10_10ms_counter_ledbar = 0;
			HAL_GPIO_WritePin(GPIOD, 0x01 << i++, 1);
		}
	}
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, 위에서부터 한칸씩 올라가면서 켜졌던 LED는 유지하면서 1개의 led가 꺼진다.
 */
void LEDbar_Keepon_Down(void)
{
	int i = 0;

	LEDbar_All_Off();
	TIM10_10ms_counter_ledbar = 0;

	while (i < 8)
	{
		if (TIM10_10ms_counter_ledbar >= 20)
		{
			TIM10_10ms_counter_ledbar = 0;
			HAL_GPIO_WritePin(GPIOD, 0x80 >> i++, 1);
		}
	}
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, led가 펴져나가는 물결 모양으로 점멸되게 한다.
 */
void LEDbar_Flower_On(void)
{
	int i = 0, delay = 30;
	TIM10_10ms_counter_ledbar = 0;

	LEDbar_All_Off();

	while(i < 4)
	{
		if (TIM10_10ms_counter_ledbar >= delay)
		{
			TIM10_10ms_counter_ledbar = 0;
			HAL_GPIO_WritePin(GPIOD, 0x01 << (4 + i), 1);
			HAL_GPIO_WritePin(GPIOD, 0x01 << (3 - i), 1);
			i++;
			delay += 10;
		}
	}
}

/*
 * desc: ledbar는 8개의 led로 구성되어 있는데, led가 모여드는 물결 모양으로 점멸되게 한다.
 */
void LEDbar_Flower_Off(void)
{
	int i = 0, delay = 70;
	TIM10_10ms_counter_ledbar = 0;

	LEDbar_All_On();

	while(i < 4)
	{
		if (TIM10_10ms_counter_ledbar >= delay)
		{
			TIM10_10ms_counter_ledbar = 0;
			HAL_GPIO_WritePin(GPIOD, 0x01 << (0 + i), 0);
			HAL_GPIO_WritePin(GPIOD, 0x01 << (7 - i), 0);
			i++;
			delay -= 10;
		}
	}
}
