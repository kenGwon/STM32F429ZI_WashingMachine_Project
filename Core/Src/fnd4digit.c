#include "fnd4digit.h"

extern volatile int fnd1ms_counter;  // fnd1ms


static void FND_update(unsigned int value);
static void FND4digit_time_update(uint32_t time);


uint16_t FND_digit[4] =
{
	FND_d1, FND_d10, FND_d100, FND_d1000
};

uint32_t FND_font[10] =
{
  FND_a|FND_b|FND_c|FND_d|FND_e|FND_f,   // 0
  FND_b|FND_c,                           // 1
  FND_a|FND_b|FND_d|FND_e|FND_g,         // 2
  FND_a|FND_b|FND_c|FND_d|FND_g,         // 3
  FND_b|FND_c|FND_f|FND_g,  			 // 4
  FND_a|FND_c|FND_d|FND_f|FND_g, 		 // 5
  FND_a|FND_c|FND_d|FND_e|FND_f|FND_g,   // 6
  FND_a|FND_b|FND_c,     				 // 7
  FND_a|FND_b|FND_c|FND_d|FND_e|FND_f|FND_g, // 8
  FND_a|FND_b|FND_c|FND_d|FND_f|FND_g    // 9
};

uint16_t FND[4];    // FND에 쓰기 위한 값을 준비하는 변수



/*
 * desc:
 */
void FND4digit_off(void)
{
#if 0 // common 에노우드 WCN4-
	HAL_GPIO_WritePin(FND_COM_PORT, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(FND_DATA_PORT,FND_font[8]|FND_p, GPIO_PIN_SET);
#else // common 캐소우드 CL5642AH30
	HAL_GPIO_WritePin(FND_COM_PORT, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(FND_DATA_PORT,FND_font[8]|FND_p, GPIO_PIN_RESET);
#endif
	return;
}

/*
 * desc:
 */
void FND4digit_on(void)
{
#if 0  // common 에노우드 WCN4-
	HAL_GPIO_WritePin(FND_COM_PORT, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(FND_DATA_PORT,FND_font[8]|FND_p, GPIO_PIN_RESET);
#else  // common 캐소우드 CL5642AH30
	HAL_GPIO_WritePin(FND_COM_PORT, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(FND_DATA_PORT,FND_font[8]|FND_p, GPIO_PIN_SET);
#endif
	return;
}

/*
 * desc:
 */
static void FND_update(unsigned int value)
{
	FND[0] = FND_font[value % 10]; // 1의 자리
	FND[1] = FND_font[value / 10 % 10]; // 10의 자리
	FND[2] = FND_font[value / 100 % 10]; // 100의 자리
	FND[3] = FND_font[value / 1000 % 10]; // 1000의 자리

	return;
}

/*
 * desc: 초 단위로 들어온 시간을 시:분:초 형태로 잘라서, 분:초만 FND 배열에 넣는다.
 * param1: 초 단위로 환산된 시간
 */
static void FND4digit_time_update(uint32_t time)
{
	uint8_t miniute, second, miniute_tens, miniute_units, second_tens, second_units;

	miniute = (time % 3600) / 60;
	second = (time % 3600) % 60;

	miniute_tens = miniute / 10;
	miniute_units = miniute % 10;
	second_tens = second / 10;
	second_units = second % 10;

	// 분
	FND[0] = FND_font[second_units]; // FND 오른쪽 끝 자리
	FND[1] = FND_font[second_tens];
	// 초
	FND[2] = FND_font[miniute_units];
	FND[3] = FND_font[miniute_tens]; // FND 왼쪽 끝 자리
}


/*
 * desc: 초 단위로 들어온 시간을 00:00(분:초) 형태로 FND에 출력한다.
 * param1: 초 단위로 환산된 시간
 */
void FND4digit_time_display(uint32_t time)
{
	static unsigned int msec=0;    // ms counter
	static int i=0;     // FND position indicator


	if (fnd1ms_counter >= 2)   // 2ms reached
	{
		fnd1ms_counter=0;
		msec += 2;

		if (msec > 1000)   // 1000ms reached
		{
			msec = 0;
			FND4digit_time_update(time);
		}
		FND4digit_off();

#if 0 // common 애노우드  WCN4-
		HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_SET);
		HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_RESET);
#else // common 캐소우드 CL5642AH30
		HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_RESET);
		HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_SET);
#endif



		i++;   // 다음 display할 FND를 가리킨다.
		i %= 4;
	}
}






void fnd4digit_sec_clock(void)
{
	static unsigned int value=0;   // 1초가 되었을때 up count
	static unsigned int msec=0;    // ms counter
	static int i=0;     // FND position indicator


	if (fnd1ms_counter >= 2)   // 2ms reached
	{
		fnd1ms_counter=0;
		msec += 2;   // 2m 주기로 FND 값을 업데이트 시키겠다는 발상이다.

		if (msec > 1000)   // 1000ms reached
		{
			msec = 0;
			value++;       // sec count를 증가
			FND_update(value);
		}

		FND4digit_off();
#if 0 // common 애노우드  WCN4-
		HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_SET);
		HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_RESET);
#else // common 캐소우드 CL5642AH30
		HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_RESET);
		HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_SET);
#endif
		i++;   // 다음 display할 FND를 가리킨다.
		i %= 4;
	}

}




void fnd4digit_main(void)
{
	unsigned int value=0;   // 1초가 되었을때 up count
	unsigned int msec=0;    // ms counter
	static int i=0;     // FND position indicator

	FND4digit_off();

	while(1)
	{
#if 1   // SYSTICK interrupt 방식
		if (fnd1ms_counter >= 2)   // 2ms reached // 2m 주기로 FND 값을 업데이트 시키겠다는 발상이다.
		{
			fnd1ms_counter=0;
			msec += 2;

			if (msec > 1000)   // 1000ms reached
			{
				msec = 0;
				value++;       // sec count를 증가
				FND_update(value);
			}

			FND4digit_off();
#if 0 // common 애노우드  WCN4-
			HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_SET);
			HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_RESET);
#else // common 캐소우드 CL5642AH30
			HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_RESET);
			HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_SET);
#endif
			i++;   // 다음 display할 FND를 가리킨다.
			i %= 4;
		}
#else   // orginal delay 방식
		msec += 8;   // 8ms
		if (msec > 1000)   // 1000ms reached
		{
			msec = 0;
			value++;       // sec count를 증가
			FND_update(value);
		}
		for (int i=0; i < 4; i++)
		{
			FND4digit_off();
			HAL_GPIO_WritePin(FND_COM_PORT,FND_digit[i], GPIO_PIN_RESET);
			HAL_GPIO_WritePin(FND_DATA_PORT, FND[i], GPIO_PIN_SET);
			HAL_Delay(2);
		}
#endif
	}
}

