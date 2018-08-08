/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：io_buf_ctrl.c
* 摘要: 输入输出缓存管理程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "app_port.h"
#include "io_buf_ctrl.h"

/*
 * 初始化IO缓冲
 * 输入: pHandle  指向缓冲句柄
 * 	   : pIOBufAddr 缓冲管理头地址
 *     : u32IOBufCnt 指示有多少个缓冲管理头
 *     : pBuf 缓冲区地址
 *     : u32BufSize 缓冲区的大小
 * 返回: 没有错误返回0
 */
s32 IOBufInit(StIOBufHandle *pHandle, 
				StIOBuf *pIOBufAddr, u32 u32IOBufCnt, 
				void *pBuf, u32 u32BufSize)
{
	u32 i;
	u32 u32EachBufSize;
	if(pHandle == NULL)
	{
		return -1;
	}
	memset(pHandle, 0, sizeof(StIOBufHandle));

	memset(pIOBufAddr, 0, sizeof(StIOBuf) * u32IOBufCnt);

	u32EachBufSize = u32BufSize / u32IOBufCnt;
	for (i = 0; i < u32IOBufCnt; i++) /* 将地址连接到缓存头上 */
	{
		pIOBufAddr[i].pBuf = pBuf;
		pBuf = (void *)((u32)pBuf + u32EachBufSize);
	}

	pHandle->pWrite = pHandle->pNeedSend = pHandle->pIOBufAddr = pIOBufAddr;

	for (i = 0; i < (u32IOBufCnt - 1); i++) /* 将每一个缓存组成一个环 */
	{
		pHandle->pIOBufAddr[i].pNext = &pHandle->pIOBufAddr[i + 1];
	}
	pHandle->pIOBufAddr[i].pNext = pHandle->pIOBufAddr;
	pHandle->u32IOBufCnt = u32IOBufCnt;
	return 0;
}

/*
 * 得到缓存指针，这个指针指向要处理的数据
 * 与函数 ReadBufEnd 成对出现
 * 输入: pHandle 指向缓冲句柄
 * 返回: NULL，没有得到指针，可能缓冲中没有数据
 */
StIOBuf *ReadBufBegin(StIOBufHandle *pHandle)
{	
	StIOBuf *pIOBuf = NULL;

	if(pHandle == NULL)
	{
		return NULL;
	}
	

	if(pHandle->u32Used == 0)
	{
		goto end;
	}
	pIOBuf = pHandle->pNeedSend;
end:

	return pIOBuf;
}

/*
 * 通知缓冲我们已经处理完这一个缓存
 * 与函数 ReadBufBegin成对出现
 * 输入: pHandle 指向缓冲句柄
 * 返回: 没有返回
 */
void ReadBufEnd(StIOBufHandle *pHandle)
{
	StIOBuf *pTmp;

	if(pHandle == NULL)
	{
		return;
	}

	if(pHandle->u32Used == 0)
	{
		return;
	}

	pTmp = pHandle->pNeedSend;		/* 复位缓存 */
	pTmp->u32Read = 0;
	pTmp->u32Used = 0;
	pHandle->pNeedSend = pHandle->pNeedSend->pNext;
	pHandle->u32Used--;
}

/*
 * 得到缓存指针，我们可以经数据写到这个指针指向的位置 
 * 与函数 WriteBufEnd 成对出现
 * 输入:  pHandle 指向缓冲句柄
 * 返回: NULL，没有得到指针，可能缓冲中没有足够的空间可以写数据
 */
StIOBuf *WriteBufBegin(StIOBufHandle *pHandle)
{	
	StIOBuf *pTmp = NULL;

	if(pHandle == NULL)
	{
		return NULL;
	}


	if(pHandle->u32Used >= pHandle->u32IOBufCnt)
	{
		goto end;
	}
	pTmp = pHandle->pWrite;
end:
	return pTmp;
}

/*
 * 通知缓冲，我们已经在缓冲中添加了一个缓存
 * 与函数 WriteBufBegin 成对出现
 * 输入:  pHandle指向缓冲句柄
 * 返回: 没有返回
 */
void WriteBufEnd(StIOBufHandle *pHandle)
{

	if(pHandle == NULL)
	{
		return;
	}


	if(pHandle->u32Used >= pHandle->u32IOBufCnt)
	{
		goto end;
	}
	pHandle->pWrite = pHandle->pWrite->pNext;
	pHandle->u32Used++;
end:
	;
}
