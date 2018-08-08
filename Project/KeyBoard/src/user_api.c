/******************(C) copyright �����XXXXX���޹�˾ **************************
* All Rights Reserved
* �ļ�����user_api.c
* ժҪ: �û���һЩAPI
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��05��
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "app_port.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "buzzer.h"

#include "user_api.h"

u32 g_u32SysTickCnt = 0;
u32 g_u32BoolIsEncode = 0;

/*
 * ˵��: ��ָ��
 * ����: ��
 * ���: ��
 */
__asm void __NOP(void)
{
  nop;
}

/*
 * ˵��: ��������
 * ����: ������
 * ���: ��
 */
void ErrorHappend(s32 s32ErrorCode)
{
	/* we can do something using the error code */

	while(1)
	{
	
	}
}

u32 SysTimeDiff( u32 u32Begin, u32 u32End)
{
	if (u32End >= u32Begin)
	{
		return (u32End - u32Begin);
	}
	else
	{
		return ((u32)(~0)) - u32Begin + u32End;
	}
}

/* �δ�ʱ���ն� */
void SysTick_Handler(void)
{
#if 1
	static u8 s_u8InnerCnt = 0;

	KeyLedFlush();/* ż��ɨ����� */		
	FlushBuzzer();

	if ((g_u32SysTickCnt & 0x0F) == 0x0F) /* 64ms */ 
	{
		u8 u8Cnt = s_u8InnerCnt & 0x03;
		if (u8Cnt == 0)
		{
			//RockFlush();
		}
		else if (u8Cnt == 1)
		{
			PushRodFlush();
		}
		else if (u8Cnt == 2)
		{
			//CodeSwitchFlush();
		}
		else if (u8Cnt == 3)
		{
			//VolumeFlush();
		}
		s_u8InnerCnt++;
	}
#endif
	g_u32SysTickCnt ++;
}

