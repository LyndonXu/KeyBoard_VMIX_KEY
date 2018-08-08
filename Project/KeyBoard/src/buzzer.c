/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：buzzer.c
* 摘要: 重要的驱动
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年05月08日
*******************************************************************************/
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "app_port.h"
#include "user_api.h"
#include "buzzer.h"

static StBuzzerCtrl s_stBuzCtrl = {false};
static EmBuzzerState s_emBuzOldState = _Buz_Close;
static EmBuzzerState s_emBuzState = _Buz_Close;

static void BuzzerPWMInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	

	TIM_TimeBaseStructInit(&TIM_InitStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	/* 1KHz */
	TIM_InitStructure.TIM_Period = 130;       
	TIM_InitStructure.TIM_Prescaler = (360-1);
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	  
	TIM_TimeBaseInit(TIM1, &TIM_InitStructure);
	TIM_ClearFlag(TIM1,TIM_FLAG_Update);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 100;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

}

static void BuzzerGPIOOpen(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = BUZZ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_50MHz;
	GPIO_Init(BUZZ_PIN_PORT, &GPIO_InitStructure);
}

static void BuzzerGPIOClose(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = BUZZ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_50MHz;
	GPIO_Init(BUZZ_PIN_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(BUZZ_PIN_PORT, BUZZ_PIN, Bit_RESET);
}


void BuzzerInit(void)
{
	//BuzzerGPIOInit();
	BuzzerPWMInit();
}

static void FlushBuzzerInner(StBuzzerCtrl *pBuzCtrl)
{
	u32 u32CurTimeDiff = SysTimeDiff(pBuzCtrl->u32BuzzStartTime, g_u32SysTickCnt);
	u32 u32NeedTimeDiff = pBuzCtrl->u16BuzzTime * pBuzCtrl->u8BuzzCnt;
	if (u32NeedTimeDiff < u32CurTimeDiff)
	{
		/* 改变蜂鸣器状态 */
		bool boState = pBuzCtrl->boCurIsBuzzing = !pBuzCtrl->boCurIsBuzzing;
		if (boState)
		{
			BUZZ_ENABLE();
		}
		else
		{
			BUZZ_DISABLE();
		}
		pBuzCtrl->u8BuzzCnt++;
	}
	if (pBuzCtrl->u8BuzzCnt > pBuzCtrl->u8BuzzDestCnt)
	{
		pBuzCtrl->boIsBuzzing = false;
		/* 关闭蜂鸣器 */
		BUZZ_DISABLE();
		s_emBuzOldState = s_emBuzState = _Buz_Close;

	}

}

void StartBuzzerInner(StBuzzerCtrl *pBuzCtrl, u8 u8Cnt, u16 u16ChangeTime)
{
	pBuzCtrl->boIsBuzzing = true;
	pBuzCtrl->u8BuzzDestCnt = u8Cnt << 1;
	pBuzCtrl->u8BuzzCnt = 1;
	pBuzCtrl->u16BuzzTime = u16ChangeTime;
	pBuzCtrl->u32BuzzStartTime = g_u32SysTickCnt;
	pBuzCtrl->boCurIsBuzzing = true;
	/* 打开蜂鸣器 */
	BUZZ_ENABLE();
}
void StartBuzzer(EmBuzzerState emState)
{
	s_emBuzState = emState;	
}

void FlushBuzzer()
{
	//if (s_emBuzState != s_emBuzOldState)
	{
		if (s_stBuzCtrl.boIsBuzzing)
		{
			switch (s_emBuzOldState)
			{
				case _Buz_Msg_Sync_Err:
				case _Buz_Msg_Sync_Finish:
				case _Buz_Msg_Set_Preset:
				case _Buz_Msg_Del_Preset:
				{
					FlushBuzzerInner(&s_stBuzCtrl);
					break;
				}
				default:
				{
					/* 关闭蜂鸣器 */
					BUZZ_DISABLE();
					s_stBuzCtrl.boIsBuzzing = false;
					s_emBuzOldState = s_emBuzState = _Buz_Close;
					break;
				}
			}
		}
		else if(s_emBuzOldState != s_emBuzState)
		{
			s_emBuzOldState = s_emBuzState;
			switch (s_emBuzOldState)
			{
				case _Buz_Msg_Sync_Err:
				{
					StartBuzzerInner(&s_stBuzCtrl, 3, 500);
					break;
				}
				case _Buz_Msg_Sync_Finish:
				{
					StartBuzzerInner(&s_stBuzCtrl, 1, 1000);
					break;
				}
				case _Buz_Msg_Set_Preset:
				{
					StartBuzzerInner(&s_stBuzCtrl, 1, 500);
					break;
				}
				case _Buz_Msg_Del_Preset:
				{
					StartBuzzerInner(&s_stBuzCtrl, 2, 500);
					break;
				}
				default:
				{
					/* 关闭蜂鸣器 */
					BUZZ_DISABLE(); 					
					s_stBuzCtrl.boIsBuzzing = false;
					s_emBuzOldState = s_emBuzState = _Buz_Close;
					break;
				}
			}
			
		}
	}
}

