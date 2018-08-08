/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：io_buf_ctrl.h
* 摘要: 输入输出缓存管理程序头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _IO_BUF_CTRL_H_
#define _IO_BUF_CTRL_H_
#include "stm32f10x_conf.h"

typedef struct _tagStIOBuf
{
	void *pBuf;
	u32 u32Read;				/* for user */
	u32 u32Used;				/* for user */
	struct _tagStIOBuf *pNext;
}StIOBuf;

typedef struct _tagStIOBufHandle
{
	StIOBuf *pIOBufAddr;
	StIOBuf *pWrite;			/* point to the position which we can write at */
	StIOBuf *pNeedSend;			/* point to the position which we can read/send at */
	u32 u32Used;				/* point out how many buffers we have used*/
	u32 u32IOBufCnt;
}StIOBufHandle;


s32 IOBufInit(StIOBufHandle *pHandle, StIOBuf *pIOBufAddr, u32 u32IOBufSize, void *pBuf, u32 u32BufSize);
StIOBuf *ReadBufBegin(StIOBufHandle *pHandle);
void ReadBufEnd(StIOBufHandle *pHandle);
StIOBuf *WriteBufBegin(StIOBufHandle *pHandle);
void WriteBufEnd(StIOBufHandle *pHandle);

#endif
