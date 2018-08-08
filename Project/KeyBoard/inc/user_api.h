/******************(C) copyright 天津市XXXXX有限公司 **************************
* All Rights Reserved
* 文件名：user_api.h
* 摘要: 用户的一些API
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月05日
*******************************************************************************/
#ifndef _USER_API_H_
#define _USER_API_H_
#include "stm32f10x_conf.h"

extern u32 g_u32SysTickCnt;
extern u32 g_u32BoolIsEncode;

extern void __NOP(void);
void ErrorHappend(s32 s32ErrorCode);
u32 SysTimeDiff( u32 u32Begin, u32 u32End);

#endif
