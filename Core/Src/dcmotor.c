#include "dcmotor.h"

void DCmotor_Forward_Rotate(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 0);
}

void DCmotor_Backward_Rotate(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
}

void DCmotor_Break(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, 1);
}




