#ifndef _WASHING__MACHINE_H_
#define _WASHING__MACHINE_H_

#include "main.h"
#include "button.h"
#include "ledbar.h"
#include "I2C_lcd.h"
#include "DHT11.h"
#include "ultrasonic.h"
#include "dcmotor.h"
#include "servomotor.h"
#include "uart_management.h"
#include "buzzer.h"

enum Laundry_Mode
{
	IDLE_MODE = 0,
	WASH_MODE,
	RINSE_MODE,
	SPIN_MODE
};

enum START_STOP_FLAG
{
	STOP = 0,
	START
};

void WashingMachine_Init(void);
void WashingMachine_Processing(void);
void WashingMachine_Terminate(void);

#endif /*_WASHING__MACHINE_H_*/
