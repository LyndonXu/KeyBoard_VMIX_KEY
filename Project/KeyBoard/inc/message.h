/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"

#include "IOCtrl.h"


void MessageUartInit(void);
StIOFIFO *MessageUartFlush(bool boSendALL);
void MessageUartRelease(StIOFIFO *pFIFO);
void MessageUartReleaseNoReleaseData(StIOFIFO *pFIFO);

int32_t MessageUartWrite(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
void MessageUARTConfig(USART_InitTypeDef *pConfig);

extern const StIOTCB c_stUartIOTCB;

#endif
