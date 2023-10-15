#include "servomotor.h"

extern TIM_HandleTypeDef htim2;

/*
 * desc: servo motor의 동작상태를 테스트 하기 위한 테스트함수
 */
void Servomotor_Test(void)
{
	while (1)
	{
		// 0도 회전
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 50);
		HAL_Delay(1000);
		// 180도 회전
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
		HAL_Delay(1000);
	}
	// 84mHz 클락을 1680으로 분주하여 50kHz로 클락 신호가 들어오게 된다.
	// T = 1/f 이므로 1 / 50000 = 0.00002s(=20us) 였다.
	// 서보모터 데이터 시트에서 2ms동안 유지되는 high 신호는 180도 회전으로 본다. ==> 0.00002 * 100개
	// 서보모터 데이터 시트에서 1.5ms동안 유지되는 high 신호는 90도 회전으로 본다. ==> 0.00002 * 75개
	// 서보모터 데이터 시트에서 1ms동안 유지되는 high 신호는 0도 회전으로 본다. ==> 0.00002 * 50개
}
