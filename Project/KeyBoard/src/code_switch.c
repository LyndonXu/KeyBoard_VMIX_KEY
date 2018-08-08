/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_swtich.c
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "app_port.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#define CANCLE_TIME			(1)

static StCodeSwitchState 	s_stCodeSwitch[CODE_SWITCH_MAX];
#define s_stCodeSwitch1		s_stCodeSwitch[0]
#define s_stCodeSwitch2		s_stCodeSwitch[1]
#define s_stCodeSwitch3		s_stCodeSwitch[2]
#define s_stCodeSwitch4		s_stCodeSwitch[3]

const u16 c_u16CodeSwitchMaxValue[CODE_SWITCH_MAX] = 
{
	CODE_SWITCH1_MAX_VALUE
};

static void CodeSwitchPinInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_2MHz;
	
	/* switch1 */
	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH1_PIN_A;
	GPIO_Init(CODE_SWITCH1_PIN_A_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH1_PIN_B;
	GPIO_Init(CODE_SWITCH1_PIN_B_PORT, &GPIO_InitStructure);


	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
	/* Connect EXTI for switch 1 */
	GPIO_EXTILineConfig(CODE_SWITCH1_INT_SRC_PORT, CODE_SWITCH1_INT_SRC);

	EXTI_InitStructure.EXTI_Line = CODE_SWITCH1_INT_LINE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	/* Enable and set switch 1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = CODE_SWITCH1_INT_CHANNEL;
	NVIC_Init(&NVIC_InitStructure);

}


void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(CODE_SWITCH1_INT_LINE) != RESET)
	{	
		bool boIsCW;
		u16 u16PinA, u16PinB;

		u16PinB = CODE_SWITCH1_PIN_B_PORT->IDR;
		u16PinA = CODE_SWITCH1_PIN_A_PORT->IDR;

		u16PinA &= CODE_SWITCH1_PIN_A;
		u16PinB &= CODE_SWITCH1_PIN_B;

		if (u16PinA == 0)
		{
			if (u16PinB == 0)
			{
				boIsCW = true;
			}
			else
			{
				boIsCW = false;
			}	
		}
		else
		{
			if (u16PinB == 0)
			{
				boIsCW = false;
			}
			else
			{
				boIsCW = true;
			}	
		}
#if CODE_SWITCH1_REVERSE
		boIsCW = !boIsCW;
#endif
		if (boIsCW)
		{
			s_stCodeSwitch1.u16Cnt++;
		}
		else
		{
			s_stCodeSwitch1.u16Cnt--;
		}
		if (s_stCodeSwitch1.u16Cnt > CODE_SWITCH1_MAX_VALUE)
		{
			if (boIsCW)
			{
				s_stCodeSwitch1.u16Cnt = 0;
			}
			else
			{
				s_stCodeSwitch1.u16Cnt = CODE_SWITCH1_MAX_VALUE;
			}
		}
		/* Clear the  pending bit */
		EXTI_ClearITPendingBit(CODE_SWITCH1_INT_LINE);
	}

}
/* 编码开关初始化,  */
void CodeSwitchInit(void)
{
	u32 i;
	CodeSwitchPinInit();

	for (i = 0; i < CODE_SWITCH_MAX; i++)
	{
		s_stCodeSwitch[i].u16Index = i;
		s_stCodeSwitch[i].u16Cnt = 0;
		s_stCodeSwitch[i].u16OldCnt = 0;		
	}
}



static bool CodeSwitchGetValueInner(StCodeSwitchState *pState)
{
	if (pState->u16Cnt != pState->u16OldCnt)
	{
		pState->u16OldCnt = pState->u16Cnt;
		return true;
	}
	return false;
}

u16 CodeSwitchPlus(u16 u16Index)
{
	USE_CRITICAL();
	u16 u16Cnt;
	if (u16Index > CODE_SWITCH_MAX)
	{
		return ~0;
	}

	u16Cnt = s_stCodeSwitch[u16Index].u16Cnt + 1;

	ENTER_CRITICAL();
	if (u16Cnt > c_u16CodeSwitchMaxValue[u16Index])
	{
		u16Cnt = 0;
	}
	s_stCodeSwitch[u16Index].u16Cnt = s_stCodeSwitch[u16Index].u16OldCnt = u16Cnt;
	EXIT_CRITICAL();
	return u16Cnt;
}


u16 CodeSwitchGetValue(u16 u16Index)
{
	return s_stCodeSwitch[u16Index].u16OldCnt;
}

u16 CodeSwitchSetValue(u16 u16Index, u16 u16Value)
{
	USE_CRITICAL();
	if (u16Index > CODE_SWITCH_MAX)
	{
		return ~0;
	}

	if (u16Value > c_u16CodeSwitchMaxValue[u16Index])
	{
		u16Value = c_u16CodeSwitchMaxValue[u16Index];
	}

	ENTER_CRITICAL();
	s_stCodeSwitch[u16Index].u16Cnt = s_stCodeSwitch[u16Index].u16OldCnt = u16Value;
	EXIT_CRITICAL();
	return u16Value;
	
}

void CodeSwitchFlush(void)
{
	u32 i;
	for (i = 0; i < CODE_SWITCH_MAX; i++)
	{
		if (CodeSwitchGetValueInner(s_stCodeSwitch + i))
		{
			StKeyMixIn stKey;
			stKey.emKeyType = _Key_CodeSwitch;
			memcpy(&(stKey.unKeyMixIn.stCodeSwitchState), s_stCodeSwitch + i, 
					sizeof(StCodeSwitchState));
			KeyBufWrite(&stKey);

		}
	}
}


