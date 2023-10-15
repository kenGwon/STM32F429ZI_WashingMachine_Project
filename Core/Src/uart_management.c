#include "uart_management.h"

extern UART_HandleTypeDef huart3; // extern from main.c // PC
extern UART_HandleTypeDef huart6; // extern from main.c // BlueTooth

extern volatile uint8_t DHT11_print_flag; // extern from DHT11.c
extern volatile uint8_t ultrasonic_print_flag; // extern from ultrasonic.c

volatile uint8_t rx_buff[QUEUE_MAX][COMMAND_LENGTH]; // UART3으로부터 수신된 char를 저장하는 버퍼 공간(\n을 만날 때까지 저장하게 할 것이다)
volatile int32_t queue_front = 0;
volatile int32_t queue_rear = 0;
volatile int32_t rx_index = 0; // rx_buff의 save위치

volatile uint8_t bt_rx_buff[COMMAND_LENGTH]; // UART6으로부터 수신된 char를 저장하는 버퍼 공간(\n을 만날 때까지 저장하게 할 것이다)
volatile int32_t bt_rx_index = 0; // bt_rx_buff의 save위치
volatile uint8_t bt_newline_detect_flag = 0; // 0은 \n을 만나지 못한 상태, 1은 \n을 만난 상태 [플래그변수]

uint8_t rx_data; // UART3 rx data
uint8_t bt_rx_data; // UART6 bluetooth(bt) rx data

/*
 * desc: UART통신으로 MCU의 RX가 외부기기의 TX로부터 1데이터 프레임(1바이트)를 수신할 때마다 HW 인터럽트를 통해 콜백되어 실행되는 함수이다.
 *       (HW와 SW의 만나는 약속장소 = call back function)
 * param1: UART 구조체
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// ComPortMaster의 TX를 통해 "ledallon\n"이 UART 통신으로 MCU의 RX로 송신되면,
	// MCU단에서는 1데이터 프레임(1바이트) 단위로 콜백 펑션이 호출되기 때문에 l, e, d, a, l, l, o, n, \n 하나하나에 이 콜백 펑션이 호출되는 것이다.
	if (huart == &huart3) // PC의 ComPortMaster와 연결된 huart3
	{
		if ((queue_rear+1)%QUEUE_MAX != queue_front)
		{
			if (rx_index < COMMAND_LENGTH)
			{
				if (rx_data == '\n' || rx_data == '\r')
				{
					rx_buff[queue_rear][rx_index] = '\0';
					rx_index = 0;
					queue_rear++;
					queue_rear %= QUEUE_MAX;
				}
				else
				{
					rx_buff[queue_rear][rx_index++] = rx_data;
				}
			}
			else
			{
				rx_index = 0;
				printf("Message Overflow !!!\n");
			}
		}
		else
		{

		}

		// 주의: 반드시 HAL_UART_Receive_IT() 함수를 call 해줘야 다음 인터럽트가 이어서 발생이 가능해진다.
		HAL_UART_Receive_IT(&huart3, &rx_data, 1);
	}

	if (huart == &huart6) // Bluetooth와 연결된 huart6
	{
		if (bt_rx_index < COMMAND_LENGTH) // 현재까지 들어온 byte가 40byte를 넘지 않으면
		{
			if(bt_rx_data == '\n' || bt_rx_data == '\r')
			{
				bt_rx_buff[bt_rx_index] = 0; // '\0'
				bt_newline_detect_flag = 1;
				bt_rx_index = 0; // 다음 메시지 저장을 위해서 rx_index값을 0으로 한다.
			}
			else
			{
				bt_rx_buff[bt_rx_index++] = bt_rx_data;
			}
		}
		else
		{
			bt_rx_index = 0;
			printf("Message Overflow !!!\n");
		}

		// 주의: 반드시 HAL_UART_Receive_IT() 함수를 call 해줘야 다음 인터럽트가 이어서 발생이 가능해진다.
		HAL_UART_Receive_IT(&huart6, &bt_rx_data, 1);
	}
}

/*
 * desc: ComPortMaster를 통해 들어온 문자열 명령에 해당하는 동작을 MCU에게 명령한다.
 */
void PC_Command_Processing(void)
{
	if (queue_front != queue_rear)
	{
		queue_front++;
		queue_front %= QUEUE_MAX;

		printf("%s\n", rx_buff[queue_front - 1]);

		// 앞에 not연산자 체크.. strcmp는 같으면 0을 리턴하기 때문에 if문에 진입하려먼 !not을 통해서 1(true)로 뒤집어 줘야 한다.
		if (!strncmp(rx_buff[queue_front - 1], "led_all_on", strlen("led_all_on")))
		{
			LEDbar_All_On();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_all_off", strlen("led_all_off")))
		{
			LEDbar_All_Off();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_on_up", strlen("led_on_up")))
		{
			LEDbar_On_Up();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_on_down", strlen("led_on_down")))
		{
			LEDbar_On_Down();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_keepon_up", strlen("led_keepon_up")))
		{
			LEDbar_Keepon_Up();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_keepon_down", strlen("led_keepon_down")))
		{
			LEDbar_Keepon_Down();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_flower_on", strlen("led_flower_on")))
		{
			LEDbar_Flower_On();
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "led_flower_off", strlen("led_flower_off")))
		{
			LEDbar_Flower_Off();
			return;
		}


//		if (!strncmp(rx_buff[queue_front - 1], "dht11time", strlen("dht11time")))
//		{
//			dht11time = atoi(rx_buff[queue_front - 1] + 9);
//			return;
//		}

		if (!strncmp(rx_buff[queue_front - 1], "dht11_on", strlen("dht11_on")))
		{
			DHT11_print_flag = 1;
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "dht11_off", strlen("dht11_off")))
		{
			DHT11_print_flag = 0;
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "ultra_on", strlen("ultra_on")))
		{
			ultrasonic_print_flag = 1;
			return;
		}
		if (!strncmp(rx_buff[queue_front - 1], "ultra_off", strlen("ultra_off")))
		{
			ultrasonic_print_flag = 0;
			return;
		}
	}
}

/*
 * desc: Bluetooth를 통해 들어온 문자열 명령에 해당하는 동작을 MCU에게 명령한다.
 */
void BT_Command_Processing(void)
{
	if (bt_newline_detect_flag)
	{
		bt_newline_detect_flag = 0;
		printf("%s\n", bt_rx_buff);

		if (!strncmp(bt_rx_buff, "ledallon", strlen("ledallon")))
		{
			LEDbar_All_On();
			return;
		}
		if (!strncmp(bt_rx_buff, "ledalloff", strlen("ledalloff")))
		{
			LEDbar_All_Off();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_on_up", strlen("led_on_up")))
		{
			LEDbar_On_Up();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_on_down", strlen("led_on_down")))
		{
			LEDbar_On_Down();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_keepon_up", strlen("led_keepon_up")))
		{
			LEDbar_Keepon_Up();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_keepon_down", strlen("led_keepon_down")))
		{
			LEDbar_Keepon_Down();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_flower_on", strlen("led_flower_on")))
		{
			LEDbar_Flower_On();
			return;
		}
		if (!strncmp(bt_rx_buff, "led_flower_off", strlen("led_flower_off")))
		{
			LEDbar_Flower_Off();
			return;
		}
	}
}
