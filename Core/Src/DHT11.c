#include "DHT11.h"

extern volatile uint32_t TIM10_10ms_counter_DHT11;

volatile uint8_t DHT11_print_flag = 1; // 0: ComPortMaster에 미출력, 1: ComPortMaster에 출력

/*
 * desc: 1.5초마다 DHT11로부터 온습도 데이터를 가져온다.
 */
void DHT11_Processing(void)
{
	uint8_t i_RH, d_RH, i_Tmp, d_Tmp;
	uint8_t lcd_buff[20];

	if (TIM10_10ms_counter_DHT11 >= 150)
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

		if (DHT11_print_flag)
		{
			printf("[Tmp]%d.%d\n", i_Tmp, d_Tmp);
			printf("[Wet]%d.%d\n", i_RH, d_RH);

			sprintf(lcd_buff, "Tmp: %d Wet: %d", i_Tmp, i_RH);
			Move_Cursor(0,0);
			LCD_String(lcd_buff);
		}
	}
}

/*
 * desc: DHT11은 active-low로 작동하기 때문에 초기화 과정에서 전위를 1로 셋팅해준다.
 */
void DHT11_Init(void)
{
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_DATA_RIN, GPIO_PIN_SET);
	HAL_Delay(3000); // 초기화 할때는 딱 한번만 일어나기 때문에 hal_delay()를 줘도 상관 없다.
}

/*
 * desc: DHT11의 데이터 시트에서 정의하고 있는대로 트리거 클락 신호를 생성한다.
 */
void DHT11_Trriger(void)
{
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_DATA_RIN, GPIO_PIN_RESET);
	delay_us(20000);
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_DATA_RIN, GPIO_PIN_SET);
	delay_us(7);
}

/*
 * desc: DHT11의 데이터 시트에서 정의하고 있는대로 트리거 신호 이후 핸드쉐이크 과정을 거친다.
 */
void DHT11_Dumi_Read(void)
{
	while( 1 == HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN) );
	while( 0 == HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN) );
	while( 1 == HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN) );
}

/*
 * desc: MCU가 DHT11로부터 데이터를 받기 위해, DHT11과 연결되어 있는 MCU의 GPIO port의 셋팅을 input 모드로 설정해준다.
 */
void DHT11_DataLine_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = DHT11_DATA_RIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; //Change Output to Input
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/*
 * desc: MCU가 DHT11에게 트리거 신호를 보내는 등의 작업을 수행하기 위해, DHT11과 연결되어 있는 MCU의 GPIO port의 셋팅을 output 모드로 설정해준다.
 */
void DHT11_DataLine_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = DHT11_DATA_RIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Change Input to Output
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/*
 * desc: DHT11로부터 들어온 전기신호를 읽어 0인지 1인지 판별한다.(DHT11 데이터 시트에서 정의한 0, 1 판별법을 코드로 구현한 것)
 */
uint8_t DHT11_rx_Data(void)
{
	uint8_t rx_data = 0;

	for(int i = 0; i < 8; i++) // 8비트 받아야 하니까 for문을 8번 돌린다.
	{
		// DHT11이 보낸 신호의 전위가 LOW인 시간은 정보로서의 의미가 없음
		while (0 == HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN))
			;

		// 위 whlie문을 탈출했다는 것은 DHT11이 보낸 신호의 전위가 HIGH로 바뀌었다는 뜻이고, 이때부터 정보로서의 의미가 있음
		delay_us(40); // 0인지 1인지 구분해야 하니까 둘의 중간 값 정도인 40마이크로 세컨드 만큼 딜레이를 주고 그 다음에 오는 if문을 통해 0인지 1인지 판단하겠다. (0이면 26마이크로 세컨드만 유지되고, 1이면 70마이크로 세컨트가 유지되기 때문)

		rx_data<<=1; // 한 비트 좌로 쉬프트 시킨다.(왜냐하면 총 8비트를 앞에서부터 쌓아가야 하기 때문)

		// 위에서 딜레이 40us를 주고 다시 읽었는데 이 if문에 걸렸다는 뜻은 HIGH 전위가 유지된 시간이 최소 40마이크로 세컨드를 넘었다는 뜻이고, 이는 곧 DHT11이 1이라는 데이터를 보낸 것으로 정의된다.
		if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN))
		{
			rx_data |= 1;
			// 단 하나의 비트만 1로 바꿔주는 것이기 때문에 그냥 상수 대입 시키는 식으로 하면 안된다.
			// 이렇게 00000000000000000000000000000001 값과 OR비트 연산을 해주면
			// rx_data는 8비트 이기 때문에 00000001과 OR 연산되어 끝자리 비트만 0에서 1로 바뀌는 효과가 발생하게 된
		}

		// DHT11이 0 신호를 보냈다면 이 while문에 들어가지도 않을 것이며, 오직 1 신호를 보낸 경우에만 위의 40마이크로 세컨드를 체크하고 남은 HIGH전위 인가 시간을 털어낼 수 있다.
		while (1 == HAL_GPIO_ReadPin(DHT11_PORT, DHT11_DATA_RIN))
			;

		// 위 while문을 탈출했다는 것은 전위가 LOW로 떨어져 다시 정보로서의 의미가 없는 구간에 돌아왔다는 것을 의미한다.
	}

	return rx_data;
}
