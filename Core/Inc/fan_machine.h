#ifndef _FAN__MACHINE_H_
#define _FAN__MACHINE_H_

#include "main.h"
#include "button.h"
#include "ledbar.h"
#include "I2C_lcd.h"

void Fan_Processing(void);
void Fan_Manual_Control(void);
void Fan_Auto_Control(void);
void Clear_Fan_LCD_Display(void);
void Fan_LCD_Display(void);
void Fan_Rotate_Direction_LED_Control(uint8_t forward_backwrd_dcmotor);
void Fan_LEDbar_Control(uint16_t ChannelControlRegister_Value);
uint16_t Get_Current_ChannelControlRegister_Value(void);

#endif /*_FAN__MACHINE_H_*/
