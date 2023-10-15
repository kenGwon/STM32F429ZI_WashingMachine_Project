#ifndef _I2C_LCD_H_
#define _I2C_LCD_H_

#include "main.h"

#define I2C_LCD_ADDRESS (0x27<<1)
#define BACKLIGHT_ON 0x08

/* LCD command   */
#define DISPLAY_ON 0x0C
#define DISPLAY_OFF 0x08
#define CLEAR_DISPLAY 0x01  //Delay 2msec
#define RETURN_HOME 0x02

void I2C_LCD_Test(void);
void LCD_Command(uint8_t command);
void LCD_Data(uint8_t data);
void I2C_LCD_Init(void);
void LCD_String(uint8_t *str);
void Move_Cursor(uint8_t row, uint8_t column);

#endif /* _I2C_LCD_H_ */
