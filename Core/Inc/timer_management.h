#ifndef _TIMER__MANAGEMENT_H_
#define _TIMER__MANAGEMENT_H_

#include "main.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void delay_us(uint32_t us);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

#endif /*_TIMER__MANAGEMENT_H_*/
