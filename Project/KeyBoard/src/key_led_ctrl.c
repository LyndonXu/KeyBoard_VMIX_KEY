/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：key_led_ctrl.c
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <string.h>

#include "key_led_ctrl.h"
#include "protocol.h"

#ifndef MAX_IO_FIFO_CNT
#define MAX_IO_FIFO_CNT 8
#endif


static StKeyMixIn s_stKeyMixIn[MAX_IO_FIFO_CNT];
static StIOFIFOList s_stIOFIFOList[MAX_IO_FIFO_CNT];
static StIOFIFOCtrl s_stIOFIFOCtrl;

void KeyBufInit(void)
{
	u32 i;
	IOFIFOInit(&s_stIOFIFOCtrl, s_stIOFIFOList, MAX_IO_FIFO_CNT, _IO_Reserved); 
	for (i = 0; i < MAX_IO_FIFO_CNT; i++)
	{
		s_stIOFIFOList[i].boNeedFree = false;
		s_stIOFIFOList[i].pData = s_stKeyMixIn + i;
		s_stIOFIFOList[i].s32Length = sizeof(StKeyMixIn);
	}
}

/* 主函数使用, 查询是否有事件, 有返回StIOFIFO.pData是 StKeyMixIn 类型指针(需转换) */
StIOFIFO *KeyBufGetBuf(void)
{
	return (StIOFIFO *)(GetAListFromRWFIFO(&s_stIOFIFOCtrl, true));
}
/* 主函数使用, KeyBufGetBuf返回值不为NULL的时候, 用此函数结束使用 */
void KeyBufGetEnd(StIOFIFO *pFIFO)
{
	ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
}


void KeyBufWrite(StKeyMixIn *pKey)
{
	StIOFIFOList *pList = NULL;
	
	if (pKey == NULL)
	{
		return;
	}

	pList = GetAUnusedFIFO(&s_stIOFIFOCtrl);
	
	if (pList != NULL)
	{
		memcpy(pList->pData, pKey, sizeof(StKeyMixIn));
		if (InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pList, true) != 0)
		{
			ReleaseAUsedFIFO(&s_stIOFIFOCtrl, pList);
		}
	}
}

