/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _MESSAGE_2_H_
#define _MESSAGE_2_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"

#include "IOCtrl.h"


void MessageUart2Init(void);
StIOFIFO *MessageUart2Flush(bool boSendALL);
void MessageUart2Release(StIOFIFO *pFIFO);
void MessageUart2ReleaseNoReleaseData(StIOFIFO *pFIFO);
int32_t MessageUart2Write(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
void MessageUART2Config(USART_InitTypeDef *pConfig);


extern const StIOTCB c_stUart2IOTCB;
#endif
