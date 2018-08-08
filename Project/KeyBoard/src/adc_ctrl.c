/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：adc_ctrl.c
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
#include "io_buf_ctrl.h"
#include "app_port.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"


#include "user_api.h"
#include "key_led_table.h"

#include "protocol.h"
#include "flash_ctrl.h"

bool g_boIsPushRodNeedReset = false;
bool g_u16IsMiniRock = false;

u16 g_u16Times = PUSH_ROD_TIMES;
u16 g_u16UpLimit = PUSH_ROD_END;
u16 g_u16DownLimit = PUSH_ROD_BEGIN;

u16 g_u16VolumeTimes = VOLUME_TIMES;
u16 g_u16VolumeUpLimit = VOLUME_END;
u16 g_u16VolumeDownLimit = VOLUME_BEGIN;


static u16 s_vu16ADCTab[ADC_GET_TOTAL * ADC_GET_CNT];
static StRockState s_stRockState;
static StPushRodState s_stPushRodState;
static StVolumeState s_stVolumeState;


const GPIO_TypeDef *c_pADCInPort[ADC_GET_TOTAL] = 
{
	ADC_PORT_1,		
	ADC_PORT_2,		
	ADC_PORT_3,		
	ADC_PORT_4,		
	ADC_PORT_5,		
};

const u16 c_u16ADCInPin[ADC_GET_TOTAL] = 
{
	ADC_PIN_1,		
	ADC_PIN_2,		
	ADC_PIN_3,		
	ADC_PIN_4,		
	ADC_PIN_5,		
};

const u8 c_u8ADCInChannel[ADC_GET_TOTAL] = 
{
	ADC_CHANNEL_1,		
	ADC_CHANNEL_2,		
	ADC_CHANNEL_3,		
	ADC_CHANNEL_4,		
	ADC_CHANNEL_5,		
};

/* ADC引脚初始化,  */
static void ADCGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	for (i = 0; i < ADC_GET_TOTAL; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16ADCInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pADCInPort[i], &GPIO_InitStructure);	
	}

}

static void ADCInit(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;
	u32 i;
	
	ADC_StructInit(&ADC_InitStructure);
	DMA_StructInit(&DMA_InitStructure);

	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADC1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)s_vu16ADCTab;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC_GET_TOTAL * ADC_GET_CNT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = ADC_GET_TOTAL;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channels configuration */ 
	for (i = 0; i < ADC_GET_TOTAL; i++)
	{
		ADC_RegularChannelConfig(ADC1, c_u8ADCInChannel[i], 
			i + 1, ADC_SampleTime_28Cycles5);
	}
	
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);  
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void RockPushRodInit(void)
{
	ADCGPIOInit();

	ADCInit();

	memset(&s_stRockState, 0, sizeof(StRockState));
	memset(&s_stPushRodState, 0, sizeof(StPushRodState));
}


static u16 ADCGetAverage(u8 u8Channel) 
{
	u32 u32Sum = 0;
	u32 i;
	u8Channel %= ADC_GET_TOTAL;
	for (i = u8Channel; i < ADC_GET_TOTAL * ADC_GET_CNT; i += ADC_GET_TOTAL)
	{
		u32Sum += s_vu16ADCTab[i];
	}
	u32Sum /= ADC_GET_CNT;
	return (u16)u32Sum;
}


/* 返回 true 时, 有数据, 否则没有数据 */
static bool RockGetValue(StRockState *pRockState)
{
	if (g_u16IsMiniRock)
	{
		u32 i;
		u16 *pValue = &(pRockState->u16RockXValue);
		u16 u16RockValue = 0;
		
		pRockState->u8RockDir = 0;
		for (i = 0; i < 2; i++)
		{
			u16RockValue = ADCGetAverage(i);

			if(u16RockValue >= ROCK_NEGATIVE_BEGIN)		/* negative */
			{	 
				if(u16RockValue >= ROCK_NEGATIVE_END)
				{
					pValue[i] = ROCK_MAX_VALUE;
				}
				else
				{
					pValue[i] = (u16RockValue - ROCK_NEGATIVE_BEGIN) / ROCK_TIMES;				
				}
				if (i == 0)
				{
					pRockState->u8RockDir |= _YNA_CAM_LEFT;
				}
				else
				{
					pRockState->u8RockDir |= _YNA_CAM_UP;
				}
			}
			else if (u16RockValue <= ROCK_POSITIVE_BEGIN)	/* positive */
			{ 	 
				if(u16RockValue <= ROCK_POSITIVE_END)
				{
					pValue[i] = ROCK_MAX_VALUE;
				}
				else
				{
					pValue[i] = (ROCK_POSITIVE_BEGIN - u16RockValue) / ROCK_TIMES;				
				}
				if (i == 0)
				{
					pRockState->u8RockDir |= _YNA_CAM_RIGHT;
				}
				else
				{
					pRockState->u8RockDir |= _YNA_CAM_DOWN;
				}
			}
			else
			{
				pValue[i] = 0;
			}
		}

		pRockState->u16RockXValue >>= 1;
		pRockState->u16RockYValue >>= 1;
		
		if (pRockState->u8RockDir == 0)
		{
			if (pRockState->u8RockOldDir != 0)
			{
				memset(pRockState, 0, sizeof(StRockState));
				return true; /* rocker stop */
			}
			return false;
		}
		if (pRockState->u8RockOldDir == pRockState->u8RockDir)
		{
			u8 u8Dir = pRockState->u8RockDir;
			bool boReturn = false;
			if ((u8Dir & (_YNA_CAM_LEFT | _YNA_CAM_RIGHT)) != 0)
			{
				if (pRockState->u16RockXValue != pRockState->u16RockXOldValue)
				{
					boReturn = true;
					pRockState->u16RockXOldValue = pRockState->u16RockXValue;
				}
			}
			if ((u8Dir & (_YNA_CAM_UP | _YNA_CAM_DOWN)) != 0)
			{
				if (pRockState->u16RockYValue != pRockState->u16RockYOldValue)
				{
					boReturn = true;
					pRockState->u16RockYOldValue = pRockState->u16RockYValue;
				}
			}
			return boReturn;
			
		}
		else
		{
			pRockState->u8RockOldDir = pRockState->u8RockDir;
		}
		return true;
	}
	else
	{
		u32 i;
		u16 *pValue = &(pRockState->u16RockXValue);
		u16 u16RockValue = 0;
		
		pRockState->u8RockDir = 0;
		for (i = 0; i < 3; i++)
		{
			u16RockValue = ADCGetAverage(i);

			if(u16RockValue <= BIG_ROCK_NEGATIVE_BEGIN)		/* negative */
			{	 
				if(u16RockValue <= BIG_ROCK_NEGATIVE_END)
				{
					pValue[i] = BIG_ROCK_MAX_VALUE;
				}
				else
				{
					pValue[i] = (BIG_ROCK_NEGATIVE_BEGIN - u16RockValue) / BIG_ROCK_TIMES;				
				}
				if (i == 0)
				{
					pRockState->u8RockDir |= _YNA_CAM_RIGHT; 
				}
				else if (i == 1)
				{
					pRockState->u8RockDir |= _YNA_CAM_DOWN;
				}
				else
				{
					pRockState->u8RockDir |= _YNA_CAM_WIDE; 				
				}
			}
			else if (u16RockValue >= BIG_ROCK_POSITIVE_BEGIN)	/* positive */
			{ 	 
				if(u16RockValue >= BIG_ROCK_POSITIVE_END)
				{
					pValue[i] = BIG_ROCK_MAX_VALUE;
				}
				else
				{
					pValue[i] = (u16RockValue - BIG_ROCK_POSITIVE_BEGIN) / BIG_ROCK_TIMES;				
				}
				if (i == 0)
				{
					pRockState->u8RockDir |= _YNA_CAM_LEFT; 
				}
				else if (i == 1)
				{
					pRockState->u8RockDir |= _YNA_CAM_UP;
				}
				else
				{
					pRockState->u8RockDir |= _YNA_CAM_TELE; 				
				}
			}
			else
			{
				pValue[i] = 0;
			}
		}

//		pRockState->u16RockXValue >>= 1;
//		pRockState->u16RockYValue >>= 1;
//		pRockState->u16RockZValue >>= 3;
		
		if (pRockState->u8RockDir == 0)
		{
			if (pRockState->u8RockOldDir != 0)
			{
				memset(pRockState, 0, sizeof(StRockState));
				return true; /* rocker stop */
			}
			return false;
		}
		if (pRockState->u8RockOldDir == pRockState->u8RockDir)
		{
			u8 u8Dir = pRockState->u8RockDir;
			bool boReturn = false;
			if ((u8Dir & (_YNA_CAM_LEFT | _YNA_CAM_RIGHT)) != 0)
			{
				if (pRockState->u16RockXValue != pRockState->u16RockXOldValue)
				{
					boReturn = true;
					pRockState->u16RockXOldValue = pRockState->u16RockXValue;
				}
			}
			if ((u8Dir & (_YNA_CAM_UP | _YNA_CAM_DOWN)) != 0)
			{
				if (pRockState->u16RockYValue != pRockState->u16RockYOldValue)
				{
					boReturn = true;
					pRockState->u16RockYOldValue = pRockState->u16RockYValue;
				}
			}
			if ((u8Dir & (_YNA_CAM_WIDE | _YNA_CAM_TELE)) != 0)
			{
				if (pRockState->u16RockZValue != pRockState->u16RockZOldValue)
				{
					boReturn = true;
					pRockState->u16RockZOldValue = pRockState->u16RockZValue;
				}
			}
			return boReturn;
			
		}
		else
		{
			pRockState->u8RockOldDir = pRockState->u8RockDir;
		}
		return true;
	}
}



u8 PushRodGetCurValue(void)
{
	return s_stPushRodState.u8PushRodValue;
}


/* */
static bool PushRodGetValue(StPushRodState *pPushRodState)
{
	u16 u16Value = ADCGetAverage(PUSH_ROD_CHANNEL);
	u16 u16OldValue = pPushRodState->u8PushRodOldValue;
	u16 u16PushRodRealValue;
#if KEYBOARD_UNION
	#define PUSH_ROD_DIFF_UNION	(4000 / (PUSH_ROD_MAX_VALUE + 1))
	if (u16Value < 48)
	{
		u16Value = 0;
	}
	else
	{
		u16Value -= 48;
	}
	if (u16Value >= 4000)
	{
		u16Value = 4000;
	}
	u16Value /= PUSH_ROD_DIFF_UNION; /* 0 ~ PUSH_ROD_MAX_VALUE */

	if (u16Value > PUSH_ROD_MAX_VALUE)
	{
		u16Value = PUSH_ROD_MAX_VALUE;
	}
	
	if (u16Value != u16OldValue)
	{
		pPushRodState->u8PushRodValue = pPushRodState->u8PushRodOldValue = u16Value;
		return true;
	}
#else
	if(u16Value >= g_u16DownLimit)
	{	 
		u16Value = 0;
	}
	else if(u16Value <= g_u16UpLimit)
	{
		u16Value = PUSH_ROD_MAX_VALUE * g_u16Times;
	}
	else
	{
		u16Value =	g_u16DownLimit - u16Value;
	}
	
	u16PushRodRealValue = pPushRodState->u16PushRodRealValue;
	if (u16PushRodRealValue > u16Value)
	{
		u16 u16Tmp = u16PushRodRealValue - u16Value;
		if (u16Tmp < (g_u16Times / 2) )
		{
			return false;
		}
	}
	else
	{
		u16 u16Tmp = u16Value - u16PushRodRealValue;
		if (u16Tmp < (g_u16Times / 2) )
		{
			return false;
		}

	}

	u16PushRodRealValue = u16Value;

	u16Value /= g_u16Times;
	if (u16OldValue != u16Value)
	{
		pPushRodState->u8PushRodValue = pPushRodState->u8PushRodOldValue = u16Value;
		pPushRodState->u16PushRodRealValue = u16Value * g_u16Times;
		return true;
	}
#endif
	return false;

}


void RockFlush(void)
{
	if (RockGetValue(&s_stRockState))
	{
		StKeyMixIn stKey;

		stKey.emKeyType = _Key_Rock;
		memcpy(&(stKey.unKeyMixIn.stRockState), &s_stRockState, 
			sizeof(StRockState));
		KeyBufWrite(&stKey);
	}
}
void PushRodFlush(void)
{
	if (PushRodGetValue(&s_stPushRodState))
	{
		StKeyMixIn stKey;

		stKey.emKeyType = _Key_Push_Rod;
		stKey.unKeyMixIn.u32PushRodValue = s_stPushRodState.u8PushRodValue;
		KeyBufWrite(&stKey);
	}
}




static bool VolumeGetValue(StVolumeState *pVolumeState)
{
	u16 u16Value = ADCGetAverage(VOLUME_CHANNEL);
	u16 u16OldValue = pVolumeState->u8VolumeOldValue;
	u16 u16VolumeRealValue = 0;
	
	if (u16Value < g_u16VolumeDownLimit)
	{
		u16Value = 0;
	}
	else
	{
		u16Value -= g_u16VolumeDownLimit;
	}
	if (u16Value > (g_u16VolumeUpLimit - g_u16VolumeDownLimit))
	{
		u16Value = (g_u16VolumeUpLimit - g_u16VolumeDownLimit);
	}
	u16VolumeRealValue = pVolumeState->u16VolumeRealValue;
	if (u16VolumeRealValue > u16Value)
	{
		u16 u16Tmp = u16VolumeRealValue - u16Value;
		if (u16Tmp < (g_u16VolumeTimes / 4) )
		{
			return false;
		}
	}
	else
	{
		u16 u16Tmp = u16Value - u16VolumeRealValue;
		if (u16Tmp < (g_u16VolumeTimes / 4) )
		{
			return false;
		}

	}
	u16VolumeRealValue = u16Value;
	u16Value /= g_u16VolumeTimes;	/* 0~100 */
	if (u16Value > VOLUME_MAX_VALUE)
	{
		u16Value = VOLUME_MAX_VALUE;
	}
	if (u16Value != u16OldValue)
	{
		pVolumeState->u8VolumeOldValue = pVolumeState->u8VolumeValue = u16Value;
		pVolumeState->u16VolumeRealValue = u16VolumeRealValue;
		return true;
	}
	return false;
	
}

u8 VolumeGetCurValue(void)
{
	return s_stVolumeState.u8VolumeOldValue;
}

void VolumeFlush(void)
{
	if (VolumeGetValue(&s_stVolumeState))
	{
		StKeyMixIn stKey;
		stKey.emKeyType = _Key_Volume;
		stKey.unKeyMixIn.u32VolumeValue = s_stVolumeState.u8VolumeValue;
		KeyBufWrite(&stKey);
	}
}


#if 1

static bool  PushRodGetTheRedressLimit(u16 u16UpLimit, u16 u16DownLimit)
{
	u16 u16Diff = 0;
	u16 u16Times = 0;
	u16 u16Ignore = 0;  

	if (u16UpLimit > u16DownLimit)
	{
		return false;
	}

	u16Diff = u16DownLimit - u16UpLimit;

	if (u16Diff < ((PUSH_ROD_MAX_VALUE + 1) * (PUSH_ROD_MIN_TIMES - 1)))
	{
		return false;
	}
	u16Times = u16Diff / (PUSH_ROD_MAX_VALUE + 1);
	
	u16Ignore = u16Diff - (u16Times * (PUSH_ROD_MAX_VALUE + 1));
	if ( u16Ignore < (PUSH_ROD_MAX_VALUE / 2))
	{
		u16Times -= 1;		
		u16Ignore = u16Diff - (u16Times * (PUSH_ROD_MAX_VALUE + 1));
	}
	
	u16Ignore /= 2;
	
	u16UpLimit += u16Ignore;

	u16DownLimit = u16UpLimit + (u16Times * (PUSH_ROD_MAX_VALUE + 1)) - 1;

	g_u16Times = u16Times;
	g_u16UpLimit = u16UpLimit;
	g_u16DownLimit = u16DownLimit;
	return true;
}

#define PR_CTRL_KEY			_Key_Effect_Ctrl_Take
#define PR_CTRL_LED			_Led_Effect_Ctrl_Take
#define PR_DOWN_KEY			_Key_Play4
#define PR_DOWN_LED			_Led_Play4
#define PR_UP_KEY			_Key_Transition4
#define PR_UP_LED			_Led_Transition4

bool RedressPushRodLimit (StIOFIFO *pFIFO)
{
	u32 u32MsgSentTime;
	u32 u32State = 0;
	u16 u16UpLimit = 0, u16DownLimit = 0;
	StKeyMixIn *pKeyIn;
	StKeyState *pKey;
	
	if (pFIFO == NULL)
	{
		return false;
	}
		
	pKeyIn = pFIFO->pData;
	if (pKeyIn == NULL)
	{
		return false;
	}

	if (pKeyIn->emKeyType != _Key_Board)
	{
		return false;
	}
	
	pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
	
	if (pKey->u8KeyValue != (u8)PR_CTRL_KEY)
	{
		return false;		
	}

	ChangeAllLedState(false);
	ChangeLedState(GET_XY(PR_CTRL_LED), true);
	
	do 
	{
		pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			break;
		}
		KeyBufGetEnd(pFIFO);
	}while(1);
	
	u32MsgSentTime = g_u32SysTickCnt;
	while(1)
	{
		StKeyState *pKey;
		
		if (SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) > 10000) /* 10S */
		{
			ChangeAllLedState(true);
			return false;
		}
		
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
		if (u32State == 0) /* get the down limit */
		{
			if (pKey->u8KeyValue == PR_DOWN_KEY)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_XY(PR_DOWN_LED), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_XY(PR_DOWN_LED), false);
					u16DownLimit = ADCGetAverage(PUSH_ROD_CHANNEL);
					u32State = 1;
				}
			}
		}
		else if (u32State == 1) /* get the up limit */
		{
			if (pKey->u8KeyValue == PR_UP_KEY)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_XY(PR_UP_LED), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_XY(PR_UP_LED), false);
					u16UpLimit = ADCGetAverage(PUSH_ROD_CHANNEL);
					break;
				}
			}			
		}
		KeyBufGetEnd(pFIFO);
	}



	if (PushRodGetTheRedressLimit(u16UpLimit, u16DownLimit))
	{
		if (WriteSaveData())
		{
			ChangeLedState(GET_XY(PR_CTRL_LED), false);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return true;
		}
	}

	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			boBlink = !boBlink;
			ChangeLedState(GET_XY(PR_CTRL_LED), boBlink);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}
	
	ChangeAllLedState(true);
	return true;
}
#endif

