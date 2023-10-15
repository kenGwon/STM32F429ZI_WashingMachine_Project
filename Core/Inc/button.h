#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "main.h" // 버튼 조작에 필요한 HAL_Driver나 GPIO 정보가 모두 main.h에 있음

#define BUTTON_RELEASE 1 // 하드웨어에서 버튼 회로를 Pull-up으로 구성해놓았기 때문에 normal-high이고, 소프트웨어에서도 형상을 일치 시키기 위해 BUTTON_RELEASE를 1로 정의함
#define BUTTON_PRESS 0 // 하드웨어에서 버튼 회로를 Pull-up으로 구성해놓았기 때문에 active-low이고, 소프트웨어에서도 형상을 일치 시키기 위해 BUTTON_PRESS를 0으로 정의함
#define BUTTON_NUMBER 5 // 하드웨어에 연결되어 있는 버튼의 갯수는 5개

int Get_Button(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t button_number);

#endif /*_BUTTON_H_*/
