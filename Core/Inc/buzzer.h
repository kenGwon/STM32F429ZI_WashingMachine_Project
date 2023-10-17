#ifndef _BUZZER_H_
#define _BUZZER_H_

#include "main.h"

enum notes
{
	C4 = 262, // 도 261.63Hz
	D4 = 294, // 레 293.66Hz
	E4 = 330, // 미 329.63Hz
	F4 = 349, // 파 349.23Hz
	G4 = 392, // 솔 392.00Hz
	A4 = 440, // 라 440.00Hz
	B4 = 494, // 시 493.88Hz
	C5 = 523,  // 도 523.25Hz
	D5 = 587
};

void noTone(void);
void buzzer_main(void);
void Mode_Start_Alarm(void);
void Mode_Complete_Alarm(void);

#endif /*_BUZZER_H_*/
