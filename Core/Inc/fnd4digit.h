#ifndef _FND4DIGIT_H_
#define _FND4DIGIT_H_

#include "main.h"

#define FND_DATA_PORT GPIOB   // data port
#define FND_COM_PORT  GPIOC   // data port

#define FND_a  GPIO_PIN_1
#define FND_b  GPIO_PIN_2
#define FND_c  GPIO_PIN_3
#define FND_d  GPIO_PIN_4
#define FND_e  GPIO_PIN_5
#define FND_f  GPIO_PIN_6
#define FND_g  GPIO_PIN_12
#define FND_p  GPIO_PIN_15

#define FND_d1000 GPIO_PIN_8
#define FND_d100  GPIO_PIN_9
#define FND_d10   GPIO_PIN_10
#define FND_d1    GPIO_PIN_11

void FND4digit_off(void);
void FND4digit_on(void);
void FND4digit_time_display(uint32_t time);
void fnd4digit_sec_clock(void);
void fnd4digit_main(void);

#endif /*_FND4DIGIT_H_*/





