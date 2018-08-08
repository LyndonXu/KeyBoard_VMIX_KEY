/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _MESSAGE_3_H_
#define _MESSAGE_3_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "IOCtrl.h"


void MessageUart3Init(void);
StIOFIFO *MessageUart3Flush(bool boSendALL);
void MessageUart3Release(StIOFIFO *pFIFO);
int32_t MessageUart3Write(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
extern const StIOTCB c_stUart3IOTCB;


#endif
