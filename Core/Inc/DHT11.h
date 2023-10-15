#ifndef _DHT11_H_
#define _DHT11_H_

#include "main.h"
#include "I2C_lcd.h"
#include "timer_management.h"

#define DHT11_PORT			GPIOA
#define DHT11_DATA_RIN		GPIO_PIN_0

void DHT11_Processing(void);
void DHT11_Init(void);
void DHT11_Trriger(void);
void DHT11_Dumi_Read(void);
void DHT11_DataLine_Input(void);
void DHT11_DataLine_Output(void);
uint8_t DHT11_rx_Data(void);

#endif /* _DHT11_H_ */
