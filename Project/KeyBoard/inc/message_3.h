/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
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
