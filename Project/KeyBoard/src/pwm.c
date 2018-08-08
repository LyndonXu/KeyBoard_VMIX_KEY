/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_swtich.c
* 摘要: PWM控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2014年11月14日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "pwm.h"
#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"


#include "user_api.h"
#include "key_led_table.h"

#include "flash_ctrl.h"

u16 g_u16LedOnLight = PWM_RESOLUTION;
u16 g_u16LedOffLight = 0;
u16 g_u16LedOn1Light = PWM_RESOLUTION;
u16 g_u16LedOn2Light = PWM_RESOLUTION;


void PWMCtrlInit()
{

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;


	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);

	ENABLE_PWM_TIMER();	

	GPIO_InitStructure.GPIO_Pin = MCU_PWM1 | MCU_PWM2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(MCU_PWM_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MCU_PWM3;
	GPIO_Init(MCU_PWM3_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MCU_PWM4;
	GPIO_Init(MCU_PWM4_PORT, &GPIO_InitStructure);
	

	TIM_TimeBaseStructure.TIM_Period = PWM_RESOLUTION - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 18;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(MCU_PWM_TIMER, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OCInitStructure.TIM_Pulse = g_u16LedOnLight;
	MCU_PWM1_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_Pulse = g_u16LedOffLight;
	MCU_PWM2_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_Pulse = g_u16LedOn1Light;
	MCU_PWM3_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_Pulse = g_u16LedOn2Light;
	MCU_PWM4_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);
	
	MCU_PWM1_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	MCU_PWM2_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	MCU_PWM3_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	MCU_PWM4_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	
	
	TIM_Cmd(MCU_PWM_TIMER, ENABLE);

}

vu16 * const pCCRAddr[_PWM_Channel_Reserved] = 
{
	&(MCU_PWM1_CCR),
	&(MCU_PWM2_CCR),
	&(MCU_PWM3_CCR),
	&(MCU_PWM4_CCR),
};

u16 PWMGetValue(s32 s32Channel)
{
	vu16 u16Tmp = ~0;
	if(s32Channel < _PWM_Channel_Reserved)
	{
		u16Tmp = pCCRAddr[s32Channel][0];
	}
	return u16Tmp;
}

void PWMPlus(s32 s32Channel)
{
	if(s32Channel < _PWM_Channel_Reserved)
	{
		vu16 u16Tmp = pCCRAddr[s32Channel][0];
		u16Tmp++;
		if (u16Tmp >= PWM_RESOLUTION)
		{
			u16Tmp = 0;
		}
		pCCRAddr[s32Channel][0] = u16Tmp;
	}
}

void PWMMinus(s32 s32Channel)
{
	if(s32Channel < _PWM_Channel_Reserved)
	{
		vu16 u16Tmp = pCCRAddr[s32Channel][0];
		u16Tmp--;
		if (u16Tmp >= PWM_RESOLUTION)
		{
			u16Tmp = PWM_RESOLUTION - 1;
		}
		pCCRAddr[s32Channel][0] = u16Tmp;
	}
}

void RedressLedLight(s32 s32Channel)
{
	if (s32Channel < _PWM_Channel_Reserved)
	{
		const u16 u16Led[_PWM_Channel_Reserved] = 
		{
			_Led_PGM_1, _Led_PGM_2, _Led_PGM_3, _Led_PGM_4,
		};
		const bool boIsOpen[_PWM_Channel_Reserved] = 
		{
			true, false, true, true,
		};
		
		StIOFIFO *pFIFO = NULL;
		StKeyMixIn *pKeyIn = NULL;

		u32 u32PlusBeginTime;
		u32 u32MinusBeginTime;

		u32 u32PlusCount;
		u32 u32MinusCount;
		
		bool boIsChange = false;
	
		ChangeAllLedState(boIsOpen[s32Channel]);
		ChangeLedState(GET_XY(u16Led[s32Channel]), !boIsOpen[s32Channel]);
		
		ChangeLedState(GET_X(_Led_RP_Up), GET_Y(_Led_RP_Up), !boIsOpen[s32Channel]);

		ChangeLedState(GET_X(_Led_RP_Down), GET_Y(_Led_RP_Down), !boIsOpen[s32Channel]);


			
		do 
		{
			pFIFO = KeyBufGetBuf();
			if (pFIFO == NULL)
			{
				break;
			}
			KeyBufGetEnd(pFIFO);
		}while(1);

		
		while(1)
		{
			StKeyState *pKey;
			pFIFO = KeyBufGetBuf();
			if (pFIFO == NULL)
			{
				continue;
			}
			
			pKeyIn = (StKeyMixIn *)(pFIFO->pData);
			if (pKeyIn == NULL)
			{
				KeyBufGetEnd(pFIFO);
				continue;
			}

			if (pKeyIn->emKeyType != _Key_Board)
			{
				KeyBufGetEnd(pFIFO);
				continue;
			}
			pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
			
			if (pKey->u8KeyValue == _Key_RP_Up)	/* light Plus */
			{
				if (pKey->u8KeyState == KEY_DOWN)
				{
					u32PlusBeginTime = g_u32SysTickCnt;
					u32PlusCount = 0;
					PWMPlus(s32Channel);
					boIsChange = true;
					ChangeLedState(GET_X(_Led_RP_Up), GET_Y(_Led_RP_Up), true);
				}
				else if (pKey->u8KeyState == KEY_KEEP)
				{
					if (SysTimeDiff(u32PlusBeginTime, g_u32SysTickCnt) > 1000)
					{
						if (u32PlusCount++ > 5)
						{
							u32PlusCount = 0;
							PWMPlus(s32Channel);
							boIsChange = true;
						}
					}
				}
				else
				{
					ChangeLedState(GET_X(_Led_RP_Down), GET_Y(_Led_RP_Down), false);
				}

			}
			else if (pKey->u8KeyValue == _Key_RP_Down)	/* light minus */
			{
				if (pKey->u8KeyState == KEY_DOWN)
				{
					u32MinusBeginTime = g_u32SysTickCnt;
					u32MinusCount = 0;
					PWMMinus(s32Channel);
					ChangeLedState(GET_X(_Led_RP_Down), GET_Y(_Led_RP_Down), true);
				}
				else if (pKey->u8KeyState == KEY_KEEP)
				{
					if (SysTimeDiff(u32MinusBeginTime, g_u32SysTickCnt) > 1000)
					{
						if (u32MinusCount++ > 5)
						{
							u32MinusCount = 0;
							PWMMinus(s32Channel);
							boIsChange = true;
						}
					}
				}
				else
				{
					ChangeLedState(GET_X(_Led_RP_Down), GET_Y(_Led_RP_Down), false);
				}
			}
			else if (pKey->u8KeyValue == _Key_RP_Return)
			{
				KeyBufGetEnd(pFIFO);
				break;
			}
			KeyBufGetEnd(pFIFO);
		}
		
		if (boIsChange)
		{
			(&g_u16LedOnLight)[s32Channel] = PWMGetValue(s32Channel);
			if (WriteSaveData())
			{
				u32 u32Time = g_u32SysTickCnt;
				ChangeLedState(GET_XY(u16Led[s32Channel]), false);
				ChangeAllLedState(false);
				while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
				ChangeAllLedState(true);
				while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
				return;
			}
		}
		{
			bool boBlink = true;
			u32 u32BlinkCnt = 0;
			while (u32BlinkCnt < 10)
			{
				u32 u32Time = g_u32SysTickCnt;
				boBlink = !boBlink;
				ChangeLedState(GET_XY(u16Led[s32Channel]), boBlink);
				while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
				u32BlinkCnt++;
			}
		}
	}
}

#if 0
void PWM1Plus(void)
{
	u16 u16Tmp = MCU_PWM1_CCR;
	u16Tmp++;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = 0;
	}
	MCU_PWM1_CCR = u16Tmp;
}


void PWM1Minus(void)
{
	u16 u16Tmp = MCU_PWM1_CCR;
	u16Tmp--;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = PWM_RESOLUTION - 1;
	}
	MCU_PWM1_CCR = u16Tmp;

}

u16 PWM2GetValue(void)
{
	return MCU_PWM2_CCR;
}

void PWM2Plus(void)
{
	u16 u16Tmp = MCU_PWM2_CCR;
	u16Tmp++;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = 0;
	}
	MCU_PWM2_CCR = u16Tmp;
}
void PWM2Minus(void)
{
	u16 u16Tmp = MCU_PWM2_CCR;
	u16Tmp--;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = PWM_RESOLUTION - 1;
	}
	MCU_PWM2_CCR = u16Tmp;

}

void RedressLedOnLight(void)
{
	StIOFIFO *pFIFO;
	StKeyMixIn *pKeyIn;

	u32 u32PlusBeginTime;
	u32 u32MinusBeginTime;

	u32 u32PlusCount;
	u32 u32MinusCount;
	
	bool boIsChange = false;
	ChangeAllLedState(true);
	ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), false);
	do 
	{
		pFIFO = KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd(pFIFO);
	}while(1);
	
	while(1)
	{
		StKeyState *pKey;
		pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			continue;
		}
		
		pKeyIn = (StKeyMixIn *)(pFIFO->pData);
		if (pKeyIn == NULL)
		{
			KeyBufGetEnd(pFIFO);
			continue;
		}

		if (pKeyIn->emKeyType != _Key_Board)
		{
			KeyBufGetEnd(pFIFO);
			continue;
		}
		pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
		
		if (pKey->u8KeyValue == _Key_PGM_NET1)	/* light Plus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32PlusBeginTime = g_u32SysTickCnt;
				u32PlusCount = 0;
				PWM1Plus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PGM_NET1), GET_Y(_Led_PGM_NET1), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32PlusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32PlusCount++ > 5)
					{
						u32PlusCount = 0;
						PWM1Plus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PGM_NET1), GET_Y(_Led_PGM_NET1), false);
			}

		}
		else if (pKey->u8KeyValue == _Key_PVW_NET1)	/* light minus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32MinusBeginTime = g_u32SysTickCnt;
				u32MinusCount = 0;
				PWM1Minus();
				ChangeLedState(GET_X(_Led_PVW_NET1), GET_Y(_Led_PVW_NET1), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32MinusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32MinusCount++ > 5)
					{
						u32MinusCount = 0;
						PWM1Minus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PVW_NET1), GET_Y(_Led_PVW_NET1), false);
			}
		}
		else if (pKey->u8KeyValue == _Key_Cutover_Auto)
		{
			KeyBufGetEnd(pFIFO);
			break;
		}
		KeyBufGetEnd(pFIFO);
	}
	
	if (boIsChange)
	{
		g_u16LedOnLight = PWM1GetValue();
		if (WriteSaveData())
		{
			u32 u32Time = g_u32SysTickCnt;
			ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), false);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return;
		}
	}
	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			u32 u32Time = g_u32SysTickCnt;
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), boBlink);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}
}

void RedressLedOffLight(void)
{
	StIOFIFO *pFIFO;
	StKeyMixIn *pKeyIn;

	u32 u32PlusBeginTime;
	u32 u32MinusBeginTime;

	u32 u32PlusCount;
	u32 u32MinusCount;
	
	bool boIsChange = false;
	ChangeAllLedState(false);
	ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), true);
	do 
	{
		pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			break;
		}
		KeyBufGetEnd(pFIFO);
	}while(1);
	while(1)
	{
		StKeyState *pKey;
		pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			continue;
		}
		
		pKeyIn = (StKeyMixIn *)(pFIFO->pData);
		if (pKeyIn == NULL)
		{
			KeyBufGetEnd(pFIFO);
			continue;
		}

		if (pKeyIn->emKeyType != _Key_Board)
		{
			KeyBufGetEnd(pFIFO);
			continue;
		}
		pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
		
		if (pKey->u8KeyValue == _Key_PGM_V4)	/* light Plus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32PlusBeginTime = g_u32SysTickCnt;
				u32PlusCount = 0;
				PWM2Plus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32PlusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32PlusCount++ > 5)
					{
						u32PlusCount = 0;
						PWM2Plus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), false);
			}

		}
		else if (pKey->u8KeyValue == _Key_PVW_V4)	/* light minus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32MinusBeginTime = g_u32SysTickCnt;
				u32MinusCount = 0;
				PWM2Minus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32MinusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32MinusCount++ > 5)
					{
						u32MinusCount = 0;
						PWM2Minus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), false);
			}
		}
		else if (pKey->u8KeyValue == _Key_DDR_Auto)
		{
			KeyBufGetEnd(pFIFO);
			break;
		}
		KeyBufGetEnd(pFIFO);
	}
	
	if (boIsChange)
	{
		g_u16LedOffLight = PWM2GetValue();
		if (WriteSaveData())
		{
			u32 u32Time = g_u32SysTickCnt;
			ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), false);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return;
		}
	}
	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			u32 u32Time = g_u32SysTickCnt;
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), boBlink);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}

	ChangeAllLedState(true);
}

#endif


