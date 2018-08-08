/******************(C) copyright 天津市XXXXX有限公司 **************************
* All Rights Reserved
* 文件名：user_api.c
* 摘要: 用户的一些API
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月05日
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
 * 说明: 空指令
 * 输入: 无
 * 输出: 无
 */
__asm void __NOP(void)
{
  nop;
}

/*
 * 说明: 错误处理函数
 * 输入: 错误码
 * 输出: 无
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

/* 滴答时钟终端 */
void SysTick_Handler(void)
{
#if 1
	static u8 s_u8InnerCnt = 0;

	KeyLedFlush();/* 偶数扫描键盘 */		
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

