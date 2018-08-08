/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：extern_io_ctrl.c
* 摘要: 外部普通IO控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2014年11月14日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "extern_io_ctrl.h"

#define IO_BYTE			((IO_CNT + 7) / 8)


static u8 s_u8ExternIO[IO_BYTE] = {0};
static u8 s_u8ExternIOBackup[IO_BYTE] = {0};

void ExternIOInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin = SHIFT_CLOCK_PIN;
	GPIO_Init(SHIFT_CLOCK_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(SHIFT_CLOCK_PORT, SHIFT_CLOCK_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = SHIFT_CLEAR_PIN;
	GPIO_Init(SHIFT_CLEAR_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(SHIFT_CLEAR_PORT, SHIFT_CLEAR_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = SHIFT_DATA_PIN;
	GPIO_Init(SHIFT_DATA_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(SHIFT_DATA_PORT, SHIFT_DATA_PIN, Bit_RESET);

}

void ExternIOClear(void)
{
	memset(s_u8ExternIO, 0, sizeof(s_u8ExternIO));
	memset(s_u8ExternIOBackup, 0, sizeof(s_u8ExternIOBackup));
	GPIO_WriteBit(SHIFT_CLEAR_PORT, SHIFT_CLEAR_PIN, Bit_RESET);
}

void ExternIOMemClear(void)
{
	memset(s_u8ExternIO, 0, sizeof(s_u8ExternIO));
}

void ExternIOCtrl(u8 u8Index, BitAction emAction)
{
	s32 i;
	u32 u32Hight, u32Low;
	if (u8Index >= IO_CNT)
	{
		return;
	}
	u32Hight = u8Index >> 3;
	u32Low = u8Index & 0x07;
	if (emAction == Bit_RESET)
	{
		s_u8ExternIO[u32Hight] &= (~(1 << u32Low));
	}
	else
	{
		s_u8ExternIO[u32Hight] |= (1 << u32Low);
	}
	
}

void ExternIOFlush(void)
{
	if (memcmp(s_u8ExternIOBackup, s_u8ExternIO, IO_BYTE) != 0)
	{
		s32 i;
		u32 u32Hight, u32Low;
		BitAction emAction;
		
		GPIO_WriteBit(SHIFT_CLEAR_PORT, SHIFT_CLEAR_PIN, Bit_SET);
		for (i = IO_CNT - 1; i >= 0; i--)
		{
			u32Hight = i >> 3;
			u32Low = i & 0x07;
			if (((s_u8ExternIO[u32Hight] >> u32Low) & 0x01) != 0)
			{
				emAction = Bit_SET;
			}
			else
			{
				emAction = Bit_RESET;
			}
			
			GPIO_WriteBit(SHIFT_DATA_PORT, SHIFT_DATA_PIN, emAction);
			GPIO_WriteBit(SHIFT_CLOCK_PORT, SHIFT_CLOCK_PIN, Bit_RESET);
			GPIO_WriteBit(SHIFT_CLOCK_PORT, SHIFT_CLOCK_PIN, Bit_SET);
		}

	
		memcpy(s_u8ExternIOBackup, s_u8ExternIO, IO_BYTE);
	}
}

