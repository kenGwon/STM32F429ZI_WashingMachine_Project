#ifndef _UART__MANAGEMENT_H_
#define _UART__MANAGEMENT_H_

#include "main.h"
#include "ledbar.h"

#define QUEUE_MAX 20
#define COMMAND_LENGTH 40

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void PC_Command_Processing(void);
void BT_Command_Processing(void);

#endif /*_UART__MANAGEMENT_H_*/
