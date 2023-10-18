#include "button.h"

// 각 버튼의 현재 상태를 저장하는 배열: 외부에서 특정 버튼의 현재 상태를 접근하고 싶다면 이 배열로 접근하면 됨ㄴ
char button_status[BUTTON_NUMBER] = {
		BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE // 버튼 5개의 최초 상태를 뗀 상태로 최기화
};

/*
 * desc: 푸시 버튼의 채터링(chattering) 현상을 고려하여, 함수가 실행된 순간 버튼의 상태가 RELEASE상태인지 PRESS상태인지 정확한 값을 전달한다.
 * param1: 함수가 접근하는 푸시 버튼 1개가 포함되어 있는 입출력 포트
 * param2: 함수가 접근하는 푸시 버튼 1개에 대응하는 1개 핀의 주소
 * param3: 함수가 접근하는 푸시 버튼 1개가 button_status[] 배열 상의 몇번째 인덱스에 해당하게 할 것인지를 결정하는 값
 * return: 푸시 버튼의 상태 (BUTTON_RELEASE 또는 BUTTON_PRESS)
 */
int Get_Button(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t button_number)
{
	unsigned char curr_state; // uint8_t로 선언해도 되지만 unsigned char로 선언한 이유는 여기에 담기는 값이 비록 0 또는 1이겠지만, 그것을 BUTTON_PRESS 또는 BUTTON_RELEASE와 같은 논리적 상태로 보겠다는 개발자의 의도를 담은 선언이다.
	curr_state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); // 0(BUTTON_PRESS) 또는 1(BUTTON_RELEASE)

	if (curr_state == BUTTON_PRESS && button_status[button_number] == BUTTON_RELEASE)
	{
		HAL_Delay(100); // 0.1초 동안 노이즈가 지나가길 기다림
		button_status[button_number] = BUTTON_PRESS; // 누르긴 눌렀으니까 우선 button_status배열 상의 상태를 BUTTON_PRESS로 바꿔준다.
		return BUTTON_RELEASE; // 아직은 버튼을 누른것으로 간주할 수 없다.(왜냐하면 button_status배열 상의 버튼의 기존 상태가 BUTTON_RELEASE였기 때문이다.)
	}
	else if (curr_state == BUTTON_RELEASE && button_status[button_number] == BUTTON_PRESS)
	{
		button_status[button_number] = BUTTON_RELEASE; // button_status배열 상의 상태를 curr_state에 맞게 다시 초기화해주고
		HAL_Delay(30);
		return BUTTON_PRESS; // 정말 버튼을 누른 것으로 인정
	}

	return BUTTON_RELEASE;
}

/*
 * 인간의 생각으로 버튼을 눌렀다는 것은 단 한번의 동작으로 이해되지만, 임베디드 시스템 프로그래밍 적으로는 그렇지 않다.
 * 우선 버튼을 눌렀다는 것은 다시 (1) 버튼을 눌렀다가 (2) 뗀다는 두가지 동작으로 구분되어야 한다.
 * 뿐만 아니라, 하드웨어 적으로 인간이 손으로 버튼을 한번 눌렀다고 해서, 소프트웨어적으로 Get_Button()함수도 단 한번만 실행되는 것이 아니다.
 * 프로그램 내에서 매우 빠르게 돌고 있는 while() 반복문 같은 곳 안에서 Get_Button()이 최소 두번 이상 호출되어야, 비로소 Get_Button()함수를 통해 BUTTON_PRESS라는 리턴 값을 받을 수 있다.
 */
