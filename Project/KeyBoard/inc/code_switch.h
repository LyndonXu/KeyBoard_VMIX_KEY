/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����code_switch.h
* ժҪ: ���뿪��ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _CODE_SWITCH_H_
#define _CODE_SWITCH_H_

#include <stdbool.h>
#include "stm32f10x_conf.h"

#define CODE_SWITCH1_PIN_A				GPIO_Pin_14
#define CODE_SWITCH1_PIN_A_PORT			GPIOC
#define CODE_SWITCH1_PIN_B				GPIO_Pin_15
#define CODE_SWITCH1_PIN_B_PORT			GPIOC


#define CODE_SWITCH1_INT_SRC			GPIO_PinSource14
#define CODE_SWITCH1_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH1_INT_LINE			EXTI_Line14
#define CODE_SWITCH1_INT_CHANNEL		EXTI15_10_IRQn



typedef struct _tagStCodeSwitchState
{
	u16 u16Index;
	
	u16 u16Cnt;
	u16 u16OldCnt;
}StCodeSwitchState;

void CodeSwitchInit(void);
u16 CodeSwitchPluse(u16 u16Index);

u16 CodeSwitchGetValue(u16 u16Index);
u16 CodeSwitchSetValue(u16 u16Index, u16 u16Value);
void CodeSwitchFlush(void);
#endif
