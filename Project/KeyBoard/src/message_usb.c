/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message_usb.c
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2017年11月18日
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "app_port.h"

#include "protocol.h"
#include "message_usb.h"

#include "hw_config.h"
#include "usb_desc.h"
#include "usb_lib.h"
#include "usb_pwr.h"



#ifndef MAX_IO_FIFO_CNT
#define MAX_IO_FIFO_CNT 16
#endif

#ifndef LEVEL_ONE_CACHE_CNT
#define LEVEL_ONE_CACHE_CNT 		32
#endif


static StIOFIFOList s_stIOFIFOList[MAX_IO_FIFO_CNT];
static StIOFIFOCtrl s_stIOFIFOCtrl;

static char s_c8LevelOneCache[LEVEL_ONE_CACHE_CNT * 2];
static StLevelOneCache s_stLevelOneCache;

static StIOFIFOList s_stIOFIFOList[MAX_IO_FIFO_CNT];
static StIOFIFOCtrl s_stIOFIFOCtrl;



void MessageUSBInit(void)
{
	/* for USB send and get a protocol message */
	IOFIFOInit(&s_stIOFIFOCtrl, s_stIOFIFOList, MAX_IO_FIFO_CNT, _IO_USB);

	/* level one cache for protocol */
	LOCInit(&s_stLevelOneCache, s_c8LevelOneCache, LEVEL_ONE_CACHE_CNT * 2);
	
	
	Set_USBPort(DISABLE); 	//USB先断开
	{
		u32 u32RedressTime = g_u32SysTickCnt;
		while(SysTimeDiff(u32RedressTime, g_u32SysTickCnt) < 100);/* 延时100ms */
	
	}
	Set_USBPort(ENABLE);	//USB再次连接
	
	USB_Interrupts_Config();

	Set_USBClock();

	USB_Init();	

	
}
void MessageUSBWriteInDate(void *pData, uint32_t u32Length)
{
	LOCWriteSomeData(&s_stLevelOneCache, pData, u32Length);
}
bool IsUSBDeviceConnect()
{
	return bDeviceState == CONFIGURED ? true : false;
}

void CopyToUSBMessage(void *pData, uint32_t u32Length, uint16_t u16ID)
{
	if (!IsUSBDeviceConnect())
	{
		return;
	}
	if (u16ID != _IO_USB_ENDP1 && u16ID != _IO_USB_ENDP2)
	{
		return;
	}
	
	if ((pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (MessageUSBWrite(pBuf, true, u16ID, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}

}


StIOFIFO *MessageUSBFlush(bool boSendALL)
{
	/* for data IN */ 
	do
	{
		char *pData = NULL;
		uint32_t u32Length = 0;
		pData = LOCCheckDataCanRead(&s_stLevelOneCache, &u32Length);
		
		if (pData == NULL)
		{
			/* no new data */
			break;
		}
		
		while(((int32_t)u32Length) >= MIDI_JACK_SIZE)
		{
			StIOFIFOList *pFIFO = NULL;
			void *pMsg = malloc(MIDI_JACK_SIZE);
			if (pMsg == NULL)
			{
				break;
			}
			/* I get some message */
			pFIFO = GetAUnusedFIFO(&s_stIOFIFOCtrl);
			if (pFIFO == NULL)
			{
				free(pMsg);
				break;
			}
			memcpy(pMsg, pData, MIDI_JACK_SIZE);
			pFIFO->pData = pMsg;
			pFIFO->s32Length = MIDI_JACK_SIZE;
			pFIFO->boNeedFree = true;
			pFIFO->u8ProtocolType = _Protocol_MIDI;
			
			InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pFIFO, true);

			u32Length -= MIDI_JACK_SIZE;
			pData += MIDI_JACK_SIZE;
		}
	} while (0);
	
	
	/* check message for send */
	do
	{
		static bool boHasSendAMessage = false;
		static StIOFIFOList stLastFIFO;
		StIOFIFOList *pFIFO = NULL;
		if (boHasSendAMessage)
		{
			/* device is online */
			if (bDeviceState == CONFIGURED)
			{
				if (stLastFIFO.u16ID == _IO_USB_ENDP1)
				{
					if (GetEPTxStatus(ENDP1) == EP_TX_NAK)
					{
						
					}
					else
					{
						if (boSendALL)
						{
							continue;	/* wait to finish to send this message */
						}
						else
						{
							break;
						}			
					}
				}
				else
				{
					if (GetEPTxStatus(ENDP2) == EP_TX_NAK)
					{
						
					}
					else
					{
						if (boSendALL)
						{
							continue;	/* wait to finish to send this message */
						}
						else
						{
							break;
						}			
					}
				}
			}
			
			if (stLastFIFO.boNeedFree)
			{
				free(stLastFIFO.pData);
			}
			boHasSendAMessage = false;
		}
		
		pFIFO = GetAListFromRWFIFO(&s_stIOFIFOCtrl, false);
		if (pFIFO != NULL)
		{
			if ((pFIFO->s32Length <= 0) || (pFIFO->pData == NULL))
			{
				if ((pFIFO->boNeedFree) && (pFIFO->pData != NULL))
				{
					free(pFIFO->pData);
				}
				if (boSendALL)
				{
					continue;	/* wait to finish to send this message */
				}
				else
				{
					break;
				}				
			}
			/* device is not online */
			if (bDeviceState != CONFIGURED)
			{
				if (pFIFO->boNeedFree)
				{
					free(pFIFO->pData);
				}
				ReleaseAUsedFIFO(&s_stIOFIFOCtrl, pFIFO);

				if (boSendALL)
				{
					continue;
				}
			}
			else
			{
				boHasSendAMessage = true;
				
				stLastFIFO = *pFIFO;
				
				if (stLastFIFO.u16ID == _IO_USB_ENDP1)
				{
					USB_SIL_Write(EP1_IN, pFIFO->pData, pFIFO->s32Length);
					SetEPTxValid(ENDP1);				
				}
				else if (stLastFIFO.u16ID == _IO_USB_ENDP2)
				{
					USB_SIL_Write(EP2_IN, pFIFO->pData, pFIFO->s32Length);
					SetEPTxValid(ENDP2);				
				}
				else
				{
					if (pFIFO->boNeedFree)
					{
						free(pFIFO->pData);
					}
					
					boHasSendAMessage = false;
				}
				
				ReleaseAUsedFIFO(&s_stIOFIFOCtrl, pFIFO);
				if (boSendALL)
				{
					continue;	/* send this message */
				}
			}
			
		}
		break;
	} while (1);
	
	/* get message for read */
	do
	{
		StIOFIFOList *pFIFO = NULL;
		pFIFO = GetAListFromRWFIFO(&s_stIOFIFOCtrl, true);
		if (pFIFO != NULL)
		{
			return (StIOFIFO *)pFIFO;
		}

	} while (0);
	
	return NULL;
}


void MessageUSBRelease(StIOFIFO *pFIFO)
{
	if (pFIFO != NULL)
	{
		if (pFIFO->boNeedFree)
		{
			free(pFIFO->pData);
		}
		ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
	}
}

int32_t MessageUSBWrite(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length)
{
	StIOFIFOList *pFIFO = NULL;
	if (!IsUSBDeviceConnect())
	{
		return -1;
	}
	if (pData == NULL)
	{
		return -1;
	}
	pFIFO = GetAUnusedFIFO(&s_stIOFIFOCtrl);
	if (pFIFO == NULL)
	{
		/* no buffer for this message */
		return -1;
	}
	pFIFO->pData = pData;
	pFIFO->s32Length = u32Length;
	pFIFO->boNeedFree = boNeedFree;
	pFIFO->u16ID = u16ID;
	InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pFIFO, false);
	
	return 0;
}

void MessageUSBReleaseNoReleaseData(StIOFIFO *pFIFO)
{
	if (pFIFO != NULL)
	{
		ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
	}
}
int32_t GetMessageUSBBufLength(void)
{
	return 0;
}


const StIOTCB c_stUSBIOTCB = 
{
	MessageUSBInit,
	MessageUSBFlush,
	MessageUSBRelease,
	MessageUSBReleaseNoReleaseData,
	GetMessageUSBBufLength,
	MessageUSBWrite,
};
