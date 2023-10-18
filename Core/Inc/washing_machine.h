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
#include "internal_rtc.h"
#include "fnd4digit.h"

#define LID_SAFTY_LIMIT 10 // cm

enum Laundry_Mode
{
	IDLE_MODE = 0,
	WASH_MODE,
	RINSE_MODE,
	SPIN_MODE
};

enum STOP_START_FLAG
{
	STOP = 0,
	START
};

enum FORWARD_BACKWARD_FLAG
{
	FORWARD = 0,
	BACKWARD
};

void WashingMachine_Init(void);
void WashingMachine_Processing(void);
void WashingMachine_Terminate(void);

#endif /*_WASHING__MACHINE_H_*/
