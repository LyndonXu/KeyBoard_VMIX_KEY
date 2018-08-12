/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：protocol.c
* 摘要: 协议控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
	
	
#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"
	
#include "buzzer.h"
	
#include "user_init.h"
#include "user_api.h"

#include "key_led_table.h"

#include "protocol.h"

#include "message.h"
#include "message_2.h"
#include "message_3.h"
#include "message_usb.h"
#include "flash_ctrl.h"
#include "extern_io_ctrl.h"

#include "common.h"

#include "usb_desc.h"

#define APP_VERSOIN_YY		17
#define APP_VERSOIN_MM		11
#define APP_VERSOIN_DD		07
#define APP_VERSOIN_V		01

#define APP_VERSOIN	"YA_VMIX_KEY_180808"

u8 g_u8CamAddr = 0;
bool g_boUsingUART2 = true;
bool g_boIsBroadcastPlay = false;
bool g_boIsDDRPlay = false;
bool g_boIsRockAus = false;
bool g_boIsRockEnable = true;

EmProtocol g_emProtocol = _Protocol_YNA;
bool g_boIsBackgroundLightEnable = true;

const u16 g_u16CamLoc[CAM_ADDR_MAX] = 
{
	0,
};


int32_t CycleMsgInit(StCycleBuf *pCycleBuf, void *pBuf, uint32_t u32Length)
{
	if ((pCycleBuf == NULL) || (pBuf == NULL) || (u32Length == 0))
	{
		return -1;
	}
	memset(pCycleBuf, 0, sizeof(StCycleBuf));
	pCycleBuf->pBuf = pBuf;
	pCycleBuf->u32TotalLength = u32Length;
	
	return 0;
}

void *CycleGetOneMsg(StCycleBuf *pCycleBuf, const char *pData, 
	uint32_t u32DataLength, uint32_t *pLength, int32_t *pProtocolType, int32_t *pErr)
{
	char *pBuf = NULL;
	int32_t s32Err = 0;
	if ((pCycleBuf == NULL) || (pLength == NULL))
	{
		s32Err = -1;
		goto end;
	}
	if (((pCycleBuf->u32TotalLength - pCycleBuf->u32Using) < u32DataLength)
		/*|| (u32DataLength == 0)*/)
	{
		PRINT_MFC("data too long\n");
		s32Err = -1;
	}
	if (u32DataLength != 0)
	{
		if (pData == NULL)
		{
			s32Err = -1;
			goto end;
		}
		else	/* copy data */
		{
			uint32_t u32Tmp = pCycleBuf->u32Write + u32DataLength;
			if (u32Tmp > pCycleBuf->u32TotalLength)
			{
				uint32_t u32CopyLength = pCycleBuf->u32TotalLength - pCycleBuf->u32Write;
				memcpy(pCycleBuf->pBuf + pCycleBuf->u32Write, pData, u32CopyLength);
				memcpy(pCycleBuf->pBuf, pData + u32CopyLength, u32DataLength - u32CopyLength);
				pCycleBuf->u32Write = u32DataLength - u32CopyLength;
			}
			else
			{
				memcpy(pCycleBuf->pBuf + pCycleBuf->u32Write, pData, u32DataLength);
				pCycleBuf->u32Write += u32DataLength;
			}
			pCycleBuf->u32Using += u32DataLength;

		}
	}

	do
	{
		uint32_t i;
		bool boIsBreak = false;

		for (i = 0; i < pCycleBuf->u32Using; i++)
		{
			uint32_t u32ReadIndex = i + pCycleBuf->u32Read;
			char c8FirstByte;
			u32ReadIndex %= pCycleBuf->u32TotalLength;
			c8FirstByte = pCycleBuf->pBuf[u32ReadIndex];
			if (c8FirstByte == ((char)0xAA))
			{
				#define YNA_NORMAL_CMD		0
				#define YNA_VARIABLE_CMD	1 /*big than PROTOCOL_YNA_DECODE_LENGTH */
				uint32_t u32MSB = 0;
				uint32_t u32LSB = 0;
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				
				/* check whether it's a variable length command */
				if (s32RemainLength >= PROTOCOL_YNA_DECODE_LENGTH - 1)
				{
					if (pCycleBuf->u32Flag != YNA_NORMAL_CMD)
					{
						u32MSB = ((pCycleBuf->u32Flag >> 8) & 0xFF);
						u32LSB = ((pCycleBuf->u32Flag >> 0) & 0xFF);
					}
					else
					{
						uint32_t u32Start = i + pCycleBuf->u32Read;
						char *pTmp = pCycleBuf->pBuf;
						if ((pTmp[(u32Start + _YNA_Mix) % pCycleBuf->u32TotalLength] == 0x04)
							&& (pTmp[(u32Start + _YNA_Cmd) % pCycleBuf->u32TotalLength] == 0x00))
						{
							u32MSB = pTmp[(u32Start + _YNA_Data2) % pCycleBuf->u32TotalLength];
							u32LSB = pTmp[(u32Start + _YNA_Data3) % pCycleBuf->u32TotalLength];
							if (s32RemainLength >= PROTOCOL_YNA_DECODE_LENGTH)
							{
								uint32_t u32Start = i + pCycleBuf->u32Read;
								uint32_t u32End = PROTOCOL_YNA_DECODE_LENGTH - 1 + i + pCycleBuf->u32Read;
								char c8CheckSum = 0;
								uint32_t j;
								char c8Tmp;
								for (j = u32Start; j < u32End; j++)
								{
									c8CheckSum ^= pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
								}
								c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
								if (c8CheckSum != c8Tmp) /* wrong message */
								{
									PRINT_MFC("get a wrong command: %d\n", u32MSB);
									pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
									pCycleBuf->u32Read += (i + 1);
									pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
									break;
								}
								
								u32MSB &= 0xFF;
								u32LSB &= 0xFF;
								
								pCycleBuf->u32Flag = ((u32MSB << 8) + u32LSB);
							}
						}
					}
				}
				u32MSB &= 0xFF;
				u32LSB &= 0xFF;
				u32MSB = (u32MSB << 8) + u32LSB;
				u32MSB += PROTOCOL_YNA_DECODE_LENGTH;
				PRINT_MFC("the data length is: %d\n", u32MSB);
				if (u32MSB > (pCycleBuf->u32TotalLength / 2))	/* maybe the message is wrong */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
					pCycleBuf->u32Read += (i + 1);
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					pCycleBuf->u32Flag = 0;
				}
				else if (((int32_t)(u32MSB)) <= s32RemainLength) /* good, I may got a message */
				{
					if (u32MSB == PROTOCOL_YNA_DECODE_LENGTH)
					{
						char c8CheckSum = 0, *pBufTmp, c8Tmp;
						uint32_t j, u32Start, u32End;
						uint32_t u32CmdLength = u32MSB;
						pBuf = (char *)malloc(u32CmdLength);
						if (pBuf == NULL)
						{
							s32Err = -1; /* big problem */
							goto end;
						}
						pBufTmp = pBuf;
						u32Start = i + pCycleBuf->u32Read;

						u32End = u32MSB - 1 + i + pCycleBuf->u32Read;
						PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
						for (j = u32Start; j < u32End; j++)
						{
							c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
							c8CheckSum ^= c8Tmp;
							*pBufTmp++ = c8Tmp;
						}
						c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
						if (c8CheckSum == c8Tmp) /* good message */
						{
							boIsBreak = true;
							*pBufTmp = c8Tmp;

							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_YNA;
							}
						}
						else
						{
							free(pBuf);
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							pCycleBuf->u32Flag = 0;
						}
					}
					else /* variable length */
					{
						uint32_t u32Start, u32End;
						uint32_t u32CmdLength = u32MSB;
						uint16_t u16CRCModBus;
						uint16_t u16CRCBuf;
						pBuf = (char *)malloc(u32CmdLength);
						if (pBuf == NULL)
						{
							s32Err = -1; /* big problem */
							goto end;
						}
						u32Start = (i + pCycleBuf->u32Read) % pCycleBuf->u32TotalLength;
						u32End = (u32MSB + i + pCycleBuf->u32Read) % pCycleBuf->u32TotalLength;
						PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
						if (u32End > u32Start)
						{
							memcpy(pBuf, pCycleBuf->pBuf + u32Start, u32MSB);
						}
						else
						{
							uint32_t u32FirstCopy = pCycleBuf->u32TotalLength - u32Start;
							memcpy(pBuf, pCycleBuf->pBuf + u32Start, u32FirstCopy);
							memcpy(pBuf + u32FirstCopy, pCycleBuf->pBuf, u32MSB - u32FirstCopy);
						}

						pCycleBuf->u32Flag = YNA_NORMAL_CMD;
						
						/* we need not check the head's check sum,
						 * just check the CRC16-MODBUS
						 */
						u16CRCModBus = CRC16((const uint8_t *)pBuf + PROTOCOL_YNA_DECODE_LENGTH, 
							u32MSB - PROTOCOL_YNA_DECODE_LENGTH - 2);
						u16CRCBuf = 0;

						LittleAndBigEndianTransfer((char *)(&u16CRCBuf), pBuf + u32MSB - 2, 2);
						if (u16CRCBuf == u16CRCModBus) /* good message */
						{
							boIsBreak = true;

							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_YNA;
							}
						}
						else
						{
							free(pBuf);
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						}
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			else if(c8FirstByte == ((char)0xFA))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_RQ_LENGTH)
				{
					volatile char c8CheckSum = 0, *pBufTmp, c8Tmp;
					volatile uint32_t j, u32Start, u32End;
					volatile uint32_t u32CmdLength = PROTOCOL_RQ_LENGTH;
					pBuf = (char *)malloc(u32CmdLength);
					if (pBuf == NULL)
					{
						s32Err = -1; /* big problem */
						goto end;
					}
					pBufTmp = pBuf;
					u32Start = i + pCycleBuf->u32Read;

					u32End = PROTOCOL_RQ_LENGTH - 1 + i + pCycleBuf->u32Read;
					PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
					for (j = u32Start; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						*pBufTmp++ = c8Tmp;
					}
					
					for (j = 2; j < PROTOCOL_RQ_LENGTH - 1; j++)
					{
						c8CheckSum += pBuf[j];
					}
					
					
					c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
					if (c8CheckSum == c8Tmp) /* good message */
					{
						boIsBreak = true;
						*pBufTmp = c8Tmp;

						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
						pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						PRINT_MFC("get a command: %d\n", u32MSB);
						PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
						*pLength = u32CmdLength;
						if (pProtocolType != NULL)
						{
							*pProtocolType = _Protocol_RQ;
						}
					}
					else
					{
						free(pBuf);
						pBuf = NULL;
						PRINT_MFC("get a wrong command: %d\n", u32MSB);
						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
						pCycleBuf->u32Read += (i + 1);
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			
			else if((c8FirstByte & 0xF0) == ((char)0x80))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_VISCA_MIN_LENGTH)
				{
					u32 j;
					u32 u32Start = i + pCycleBuf->u32Read;
					u32 u32End = pCycleBuf->u32Using + pCycleBuf->u32Read;
					char c8Tmp = 0;
					for (j = u32Start + PROTOCOL_VISCA_MIN_LENGTH - 1; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						if (c8Tmp == (char)0xFF)
						{
							u32End = j;
							break;
						}
					}
					if (c8Tmp == (char)0xFF)
					{
						/* wrong message */
						if ((u32End - u32Start + 1) > PROTOCOL_VISCA_MAX_LENGTH)
						{
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;							
						}
						else
						{
							char *pBufTmp;
							uint32_t u32CmdLength = u32End - u32Start + 1;
							pBuf = (char *)malloc(u32CmdLength);
							if (pBuf == NULL)
							{
								s32Err = -1; /* big problem */
								goto end;
							}	
							pBufTmp = pBuf;
							boIsBreak = true;
							
							PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
							for (j = u32Start; j <= u32End; j++)
							{
								*pBufTmp++ = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
							}
							
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_VISCA;
							}

						}
					}
					else
					{
						/* wrong message */
						if ((u32End - u32Start) >= PROTOCOL_VISCA_MAX_LENGTH)
						{
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;							
						}
						else
						{
							pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
							pCycleBuf->u32Read += i;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							boIsBreak = true;						
						}
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			else if(c8FirstByte == ((char)0xA5))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_SB_LENGTH)
				{
					volatile char c8CheckSum = 0, *pBufTmp, c8Tmp;
					volatile uint32_t j, u32Start, u32End;
					volatile uint32_t u32CmdLength = PROTOCOL_SB_LENGTH;
					pBuf = (char *)malloc(u32CmdLength);
					if (pBuf == NULL)
					{
						s32Err = -1; /* big problem */
						goto end;
					}
					pBufTmp = pBuf;
					u32Start = i + pCycleBuf->u32Read;

					u32End = PROTOCOL_SB_LENGTH - 1 + i + pCycleBuf->u32Read;
					PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
					for (j = u32Start; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						*pBufTmp++ = c8Tmp;
					}
					
					for (j = 1; j < PROTOCOL_SB_LENGTH - 1; j++)
					{
						c8CheckSum += pBuf[j];
					}
					
					
					c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
					if (c8CheckSum == c8Tmp) /* good message */
					{
						boIsBreak = true;
						*pBufTmp = c8Tmp;

						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
						pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						PRINT_MFC("get a command: %d\n", u32MSB);
						PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
						*pLength = u32CmdLength;
						if (pProtocolType != NULL)
						{
							*pProtocolType = _Protocol_SB;
						}
					}
					else
					{
						free(pBuf);
						pBuf = NULL;
						PRINT_MFC("get a wrong command: %d\n", u32MSB);
						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
						pCycleBuf->u32Read += (i + 1);
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
		}
		if ((i == pCycleBuf->u32Using) && (!boIsBreak))
		{
			PRINT_MFC("cannot find AA, i = %d\n", pCycleBuf->u32Using);
			pCycleBuf->u32Using = 0;
			pCycleBuf->u32Read += i;
			pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
			pCycleBuf->u32Flag = 0;
		}

		if (boIsBreak)
		{
			break;
		}
	} while (((int32_t)pCycleBuf->u32Using) > 0);

	//if (pCycleBuf->u32Write + u32DataLength)

end:
	if (pErr != NULL)
	{
		*pErr = s32Err;
	}
	return pBuf;
}
void *YNAMakeAnArrayVarialbleCmd(uint16_t u16Cmd, void *pData,
	uint32_t u32Count, uint32_t u32Length, uint32_t *pCmdLength)
{
	uint32_t u32CmdLength;
	uint32_t u32DataLength;
	uint32_t u32Tmp;
	uint8_t *pCmd = NULL;
	uint8_t *pVarialbleCmd;
	if (pData == NULL)
	{
		return NULL;
	}
	
	u32DataLength = u32Count * u32Length;
	
	/*  */
	u32CmdLength = PROTOCOL_YNA_DECODE_LENGTH + 6 + u32DataLength + 2;
	pCmd = malloc(u32CmdLength);
	if (pCmd == NULL)
	{
		return NULL;
	}
	memset(pCmd, 0, u32CmdLength);
	pCmd[_YNA_Sync] = 0xAA;
	pCmd[_YNA_Mix] = 0x04;
	pCmd[_YNA_Cmd] = 0x00;
	
	/* total length */
	u32Tmp = u32CmdLength - PROTOCOL_YNA_DECODE_LENGTH;
	LittleAndBigEndianTransfer((char *)pCmd + _YNA_Data2, (const char *)(&u32Tmp), 2);
	
	YNAGetCheckSum(pCmd);
	
	pVarialbleCmd = pCmd + PROTOCOL_YNA_DECODE_LENGTH;
	
	/* command serial */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd, (const char *)(&u16Cmd), 2);

	/* command count */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 2, (const char *)(&u32Count), 2);

	/* Varialble data length */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 4, (const char *)(&u32Length), 2);
	
	/* copy the data */
	memcpy(pVarialbleCmd + 6, pData, u32DataLength);

	/* get the CRC16 of the variable command */
	u32Tmp = CRC16(pVarialbleCmd, 6 + u32DataLength);
	
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 6 + u32DataLength, 
		(const char *)(&u32Tmp), 2);

	if (pCmdLength != NULL)
	{
		*pCmdLength = u32CmdLength;
	}
	
	return pCmd;
}

void *YNAMakeASimpleVarialbleCmd(uint16_t u16Cmd, void *pData, 
	uint32_t u32DataLength, uint32_t *pCmdLength)
{
	return YNAMakeAnArrayVarialbleCmd(u16Cmd, pData, 1, u32DataLength, pCmdLength);
}

void CopyToUart1Message(void *pData, u32 u32Length)
{
	if ((pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (MessageUartWrite(pBuf, true, _IO_Reserved, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}

}
void CopyToUart3Message(void *pData, u32 u32Length)
{
	if ((pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (MessageUart3Write(pBuf, true, _IO_Reserved, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}

}

void CopyToUartMessage(void *pData, u32 u32Length)
{
	CopyToUart1Message(pData, u32Length);
	//CopyToUart3Message(pData, u32Length);
}
void CopyToUart2Message(void *pData, u32 u32Length)
{
}


u8 u8YNABuf[PROTOCOL_YNA_ENCODE_LENGTH];

int32_t BaseCmdProcess(StIOFIFO *pFIFO, const StIOTCB *pIOTCB)
{
	uint8_t *pMsg;
	bool boGetVaildBaseCmd = true;
	if (pFIFO == NULL)
	{
		return -1;
	}
	pMsg = (uint8_t *)pFIFO->pData;
	
	if (pMsg[_YNA_Sync] != 0xAA)
	{
		return -1;
	}
	
	if (pMsg[_YNA_Mix] == 0x0C)
	{
		if (pMsg[_YNA_Cmd] == 0x80)
		{
			uint8_t u8EchoBase[PROTOCOL_YNA_DECODE_LENGTH] = {0};
			uint8_t *pEcho = NULL;
			uint32_t u32EchoLength = 0;
			bool boHasEcho = true;
			bool boNeedCopy = true;
			bool boNeedReset = false;
			u8EchoBase[_YNA_Sync] = 0xAA;
			u8EchoBase[_YNA_Mix] = 0x0C;
			u8EchoBase[_YNA_Cmd] = 0x80;
			u8EchoBase[_YNA_Data1] = 0x01;
			switch(pMsg[_YNA_Data3])
			{
				case 0x01:	/* just echo the same command */
				{
					//SetOptionByte(OPTION_UPGRADE_DATA);
					//boNeedReset = true;
				}
				case 0x02:	/* just echo the same command */
				{
					u8EchoBase[_YNA_Data3] = pMsg[_YNA_Data3];
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					if (pEcho == NULL)
					{
						boHasEcho = false;
						break;
					}
					u32EchoLength = PROTOCOL_YNA_DECODE_LENGTH;						
					break;
				}
				case 0x03:	/* return the UUID */
				{
					StUID stUID;
					GetUID(&stUID);
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x8003, 
							&stUID, sizeof(StUID), &u32EchoLength);
					break;
				}
				case 0x05:	/* return the BufLength */
				{
					uint16_t u16BufLength = 0;
					if (pIOTCB != NULL
						 && pIOTCB->pFunGetMsgBufLength != 0)
					{
						u16BufLength = pIOTCB->pFunGetMsgBufLength();
					}
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x8005, 
							&u16BufLength, sizeof(uint16_t), &u32EchoLength);
					break;
				}
				case 0x09:	/* RESET the MCU */
				{
					NVIC_SystemReset();
					boHasEcho = false;
					break;
				}
				case 0x0B:	/* Get the application's CRC32 */
				{
					uint32_t u32CRC32 = ~0;
					u32CRC32 = AppCRC32(~0);
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x800B, 
							&u32CRC32, sizeof(uint32_t), &u32EchoLength);
					break;
				}
				case 0x0C:	/* get the version */
				{
					const char *pVersion = APP_VERSOIN;
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x800C, 
							(void *)pVersion, strlen(pVersion) + 1, &u32EchoLength);
					break;
				}
				
				default:
					boHasEcho = false;
					boGetVaildBaseCmd = false;
					break;
			}
			if (boHasEcho && pEcho != NULL)
			{
				if (boNeedCopy)
				{
					YNAGetCheckSum(u8EchoBase);
					memcpy(pEcho, u8EchoBase, PROTOCOL_YNA_DECODE_LENGTH);
				}
				if (pIOTCB == NULL)
				{
					free(pEcho);
				}
				else if (pIOTCB->pFunMsgWrite == NULL)
				{
					free(pEcho);			
				}
				else if(pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
				{
					free(pEcho);
				}
			}
			
			/* send all the command in the buffer */
			if (boNeedReset)
			{
				//MessageUartFlush(true); 
				NVIC_SystemReset();
			}
		}
	}
	else if (pMsg[_YNA_Mix] == 0x04)	/* variable command */
	{
		uint32_t u32TotalLength = 0;
		uint32_t u32ReadLength = 0;
		uint8_t *pVariableCmd;
		
		boGetVaildBaseCmd = false;
		
		/* get the total command length */
		LittleAndBigEndianTransfer((char *)(&u32TotalLength), (const char *)pMsg + _YNA_Data2, 2);
		pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH;
		u32TotalLength -= 2; /* CRC16 */
		while (u32ReadLength < u32TotalLength)
		{
			uint8_t *pEcho = NULL;
			uint32_t u32EchoLength = 0;
			bool boHasEcho = true;
			uint16_t u16Command = 0, u16Count = 0, u16Length = 0;
			LittleAndBigEndianTransfer((char *)(&u16Command),
				(char *)pVariableCmd, 2);
			LittleAndBigEndianTransfer((char *)(&u16Count),
				(char *)pVariableCmd + 2, 2);
			LittleAndBigEndianTransfer((char *)(&u16Length),
				(char *)pVariableCmd + 4, 2);

			switch (u16Command)
			{
				case 0x800A:
				{
					/* check the crc32 and UUID, and BTEA and check the number */
					int32_t s32Err;
					char *pData = (char *)pVariableCmd + 6;
					StBteaKey stLic;
					StUID stUID;
					uint32_t u32CRC32;
					
					GetUID(&stUID);
					u32CRC32 = AppCRC32(~0);
					GetLic(&stLic, &stUID, u32CRC32, true);
					
					if (memcmp(&stLic, pData, sizeof(StBteaKey)) == 0)
					{
						s32Err = 0;
						
						WriteLic(&stLic, true, 0);
					}
					else
					{
						s32Err = -1;
					}
					pEcho = YNAMakeASimpleVarialbleCmd(0x800A, &s32Err, 4, &u32EchoLength);
					boGetVaildBaseCmd = true;
					break;
				}
				default:
					break;
			}
			
			if (boHasEcho && pEcho != NULL)
			{
				if (pIOTCB == NULL)
				{
					free(pEcho);
				}
				else if (pIOTCB->pFunMsgWrite == NULL)
				{
					free(pEcho);
				}
				else if(pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
				{
					free(pEcho);
				}
			}
			
			
			u32ReadLength += (6 + (uint32_t)u16Count * u16Length);
			pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH + u32ReadLength;
		}
	}
	else
	{
		boGetVaildBaseCmd = false;
	}

	return boGetVaildBaseCmd ? 0: -1;
}


void GlobalStateInit(void)
{
	g_u8CamAddr = 0;
	
	g_boUsingUART2 = true;
	g_boIsPushRodNeedReset = false;
	g_boIsBroadcastPlay = false;
	g_boIsDDRPlay = false;
	g_boIsRockEnable = true;	
}


void ChangeEncodeState(void)
{

}

void YNADecode(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}

}
void YNAEncodeAndGetCheckSum(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}
}


void YNAGetCheckSum(u8 *pBuf)
{
	s32 i, s32End;
	u8 u8Sum = pBuf[0];

	if (g_u32BoolIsEncode)
	{
		s32End = PROTOCOL_YNA_ENCODE_LENGTH - 1;	
	}
	else
	{
		s32End = PROTOCOL_YNA_DECODE_LENGTH - 1;
	}
	for (i = 1; i < s32End; i++)
	{
		u8Sum ^= pBuf[i];
	}
	pBuf[i] = u8Sum;
}

void PelcoDGetCheckSum(u8 *pBuf)
{
	s32 i;
	u8 u8Sum = 0;
	for (i = 1; i < 6; i++)
	{
		u8Sum += pBuf[i];
	}
	pBuf[i] = u8Sum;
}

void SBGetCheckSum(u8 *pBuf)
{
	s32 i;
	u8 u8Sum = 0;
	for (i = 1; i < PROTOCOL_SB_LENGTH - 1; i++)
	{
		u8Sum += pBuf[i];
	}
	pBuf[i] = u8Sum;
}

bool ProtocolSelect(StIOFIFO *pFIFO)
{
	const u8 u8KeyMap[] = 
	{
		_Key_PGM_1, _Key_PGM_2
	};
	const u16 u16LedMap[] = 
	{
		_Led_PGM_1, _Led_PGM_2
	};
	
	u32 u32MsgSentTime;
	u32 u32State = 0;
	StKeyMixIn *pKeyIn;
	StKeyState *pKey;
	
	if (pFIFO == NULL)
	{
		return false;
	}
		
	pKeyIn = pFIFO->pData;
	if (pKeyIn == NULL)
	{
		return false;
	}

	if (pKeyIn->emKeyType != _Key_Board)
	{
		return false;
	}
	
	pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
	if (pKey->u8KeyValue != (u8)_Key_PVW_1)
	{
		return false;		
	}


	ChangeAllLedState(false);
	
	u32MsgSentTime = g_u32SysTickCnt;
	while(1)
	{
		if (pKeyIn != NULL)
		{
			pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
			if (u32State == 0)
			{
				if (pKey->u8KeyValue == (u8)_Key_PVW_1)
				{
					u32MsgSentTime = g_u32SysTickCnt;
					if (pKey->u8KeyState == KEY_DOWN)
					{
						ChangeAllLedState(false);
						ChangeLedState(GET_XY(_Led_PVW_1), true);
					}
					else if (pKey->u8KeyState == KEY_UP)
					{
						//ChangeLedState(LED(_Led_Switch_Video_Bak5), false);
						u32State = 1;
					}
				}			
			}
			else if (u32State == 1) /* get protocol or bandrate */
			{
				u32 u32Index = ~0;
				u32 i;
				for (i = 0; i < sizeof(u8KeyMap); i++)
				{
					if (pKey->u8KeyValue == u8KeyMap[i])
					{
						u32Index = i;
						break;
					}						
				}
				if (u32Index != ~0)
				{
					u32MsgSentTime = g_u32SysTickCnt;
					if (pKey->u8KeyState == KEY_DOWN)
					{
						ChangeLedState(GET_XY(u16LedMap[u32Index]), true);
					}
					else if (pKey->u8KeyState == KEY_UP)
					{
						ChangeLedState(GET_XY(u16LedMap[u32Index]), false);
						u32State = 2 + u32Index;
						break;
					}
				}
			}
		}
		
		KeyBufGetEnd(pFIFO);
		
		pFIFO = NULL;
		
		if (SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) > 5000) /* 10S */
		{
			ChangeAllLedState(false);
			return false;
		}

		
		pKeyIn = NULL;
		pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			continue;
		}
		
		pKeyIn = pFIFO->pData;
		if (pKeyIn == NULL)
		{
			KeyBufGetEnd(pFIFO);
			continue;
		}

		if (pKeyIn->emKeyType != _Key_Board)
		{
			pKeyIn = NULL;
			KeyBufGetEnd(pFIFO);
			continue;
		}	
	}
	
	
	KeyBufGetEnd(pFIFO);
	
	
	if (u32State >= 2)
	{
		switch (u32State)
		{
			case 2:
			{
				g_emProtocol = _Protocol_YNA;
				break;
			}
			case 3:
			{
				g_emProtocol = _Protocol_SB;
				break;
			}
			default:
				break;
		}
		
		
		if (WriteSaveData())
		{
			u32MsgSentTime = g_u32SysTickCnt;
			ChangeAllLedState(true);
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1500);/* 延时1s */
			ChangeAllLedState(false);
			return true;
		}

		{
			bool boBlink = true;
			u32 u32BlinkCnt = 0;
			while (u32BlinkCnt < 10)
			{
				boBlink = !boBlink;
				ChangeLedState(GET_XY(_Led_PVW_1), boBlink);
				u32MsgSentTime = g_u32SysTickCnt;
				while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 100);/* 延时1s */
				u32BlinkCnt++;
			}
		}	
	}
	
	ChangeAllLedState(false);
	return false;
}


void TallyUartSend(u8 Tally1, u8 Tally2)
{
	u32 i, j;
	u8 u8Buf[4] = {0xFC, 0x00, 0x00, 0x00};

	for (j = 0; j < 2; j++)
	{	
		for(i = 0; i < 4; i++)
		{
			if (((Tally1 >> (4 * j + i)) & 0x01) != 0x00)
			{
				u8Buf[1 + j] |= (1 << (i * 2));
			}
			
			if (((Tally2 >> (4 * j + i)) & 0x01) != 0x00)
			{
				u8Buf[1 + j] |= (1 << (i * 2 + 1));			
			}			
		}	
	}
	
	u8Buf[3] = u8Buf[0] ^ u8Buf[1] ^ u8Buf[2];
	
	CopyToUart3Message(u8Buf, 4);
}
u8 g_u8YNATally[2] = {0, 0};

void SetTallyPGM(u8 u8Index, bool boIsLight, bool boIsClear, bool boIsSend)
{
	if (u8Index > 7)
	{
		return;
	}
	if (boIsClear)
	{
		g_u8YNATally[0] = 0;
	}
	if (boIsLight)
	{
		g_u8YNATally[0] |= (1 << u8Index);
	}
	else 
	{
		g_u8YNATally[0] &= ~(1 << u8Index);					
	}
	if (boIsSend)
	{
		TallyUartSend(g_u8YNATally[0], g_u8YNATally[1]);					
	}
	
}

void SetTallyPVW(u8 u8Index, bool boIsLight, bool boIsClear, bool boIsSend)
{
	if (u8Index > 7)
	{
		return;
	}
	if (boIsClear)
	{
		g_u8YNATally[1] = 0;
	}
	if (boIsLight)
	{
		g_u8YNATally[1] |= (1 << u8Index);
	}
	else 
	{
		g_u8YNATally[1] &= ~(1 << u8Index);					
	}
	if (boIsSend)
	{
		TallyUartSend(g_u8YNATally[0], g_u8YNATally[1]);					
	}
	
}

static void ChangeLedArrayState(const u16 *pLed, u16 u16Cnt, bool boIsLight)
{
	u16 i;
	for (i = 0; i < u16Cnt; i++)
	{
		ChangeLedStateWithBackgroundLight(GET_XY(pLed[i]), boIsLight);
	}
}
static void ChangeLedArrayStateNoBackgroundLight(const u16 *pLed, u16 u16Cnt, bool boIsLight)
{
	u16 i;
	for (i = 0; i < u16Cnt; i++)
	{
		ChangeLedState(GET_XY(pLed[i]), boIsLight);
	}
}

static bool KeyBoardProcess(StKeyMixIn *pKeyIn)
{
	u32 i;
	for (i = 0; i < pKeyIn->u32Cnt; i++)
	{
		u8 *pBuf;
		StKeyState *pKeyState = pKeyIn->unKeyMixIn.stKeyState + i;
		u8 u8KeyValue = pKeyState->u8KeyValue;
		if (pKeyState->u8KeyState == KEY_KEEP)
		{
			if (u8KeyValue != _Key_PowerDown)
				continue;
		}

		pBuf = u8YNABuf;

		memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

		pBuf[_YNA_Sync] = 0xAA;
		pBuf[_YNA_Addr] = g_u8CamAddr;
		pBuf[_YNA_Mix] = 0x07;
		if (pKeyState->u8KeyState == KEY_UP)
		{
			pBuf[_YNA_Data1] = 0x01;
		}

		/* 处理按键 */
		switch (u8KeyValue)
		{
			case _Key_QuickPlay1:
			case _Key_QuickPlay2:
			case _Key_QuickPlay3:
			case _Key_QuickPlay4:
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_QuickPlay1 + 0x60;	
				break;
			}
			
			case _Key_Loop1:	
			case _Key_Loop2:			
			case _Key_Loop3:			
			case _Key_Loop4:	
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Loop1 + 0x70;	
				break;
			}

			case _Key_FullScreen:
			case _Key_MultiCorder:
			case _Key_PlayList:
			case _Key_StartExternal:
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_FullScreen + 0x53;	
				break;
			}
			
			case _Key_Recording:
			case _Key_Stream:
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Recording + 0x50;					
				break;
			}
			
			
			case _Key_PGM_1:
			case _Key_PGM_2:
			case _Key_PGM_3:
			case _Key_PGM_4:
			case _Key_PGM_5:
			case _Key_PGM_6:
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_PGM_1 + 0x02;			
				break;
			}
			case _Key_PGM_7:
			case _Key_PGM_8:
			case _Key_PGM_9:
			case _Key_PGM_10:
			case _Key_PGM_11:
			case _Key_PGM_12:
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_PGM_7 + 0x80;			
				break;
			}


			case _Key_PVW_1:
			case _Key_PVW_2:
			case _Key_PVW_3:
			case _Key_PVW_4:
			case _Key_PVW_5:
			case _Key_PVW_6:
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_PVW_1 + 0x08;			
				break;
			}
			case _Key_PVW_7:
			case _Key_PVW_8:
			case _Key_PVW_9:
			case _Key_PVW_10:
			case _Key_PVW_11:
			case _Key_PVW_12:	
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_PVW_7 + 0x90;			
				break;
			}
		
			case _Key_Effect_Ctrl_Take:
			case _Key_Effect_Ctrl_Cut:
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Effect_Ctrl_Take;	
				break;
			}
			case _Key_Overlay1:				
			case _Key_Overlay2:
			case _Key_Overlay3:
			case _Key_Overlay4:
			case _Key_Overlay5:
			case _Key_Overlay6:
			case _Key_Overlay7:
			case _Key_Overlay8:
			case _Key_Overlay9:
			case _Key_Overlay10:
			case _Key_Overlay11:
			case _Key_Overlay12:		
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Overlay1 + 0x50;	
				break;
			}


			case _Key_FTB:
			{
				pBuf[_YNA_Cmd] = 0x4A;
				pBuf[_YNA_Data2] = 0x04;	
				break;
			}
			case _Key_QuickPlay:
			{
				pBuf[_YNA_Cmd] = 0x4A;
				pBuf[_YNA_Data2] = 0x08;	
				break;
			}
			case _Key_Dsk1:
			case _Key_Dsk2:
			case _Key_Dsk3:
			case _Key_Dsk4:
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Dsk1 + 0x80;					
				break;
			}

			case _Key_Play1:
			case _Key_Play2:
			case _Key_Play3:
			case _Key_Play4:
			{
				pBuf[_YNA_Cmd] = 0x47;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Play1 + 0x90;					
				break;
			}

			case _Key_Transition1:
			case _Key_Transition2:
			case _Key_Transition3:
			case _Key_Transition4:
			{
				pBuf[_YNA_Cmd] = 0x48;
				pBuf[_YNA_Data2] = u8KeyValue - _Key_Transition1 + 0x60;	
				break;
			}
			
			case _Key_PowerDown:
			{
				static u32 u32KeyDownTime = 0;
				static bool boIsKeyDown = false;
				if (pKeyState->u8KeyState == KEY_DOWN)
				{
					boIsKeyDown = true;
					u32KeyDownTime = g_u32SysTickCnt;
				}
				else if (pKeyState->u8KeyState == KEY_KEEP)		
				{
					if (SysTimeDiff(u32KeyDownTime, g_u32SysTickCnt) > 2000)
					{
						ChangeAllLedState(false);						
					}
				}
				else if (boIsKeyDown && SysTimeDiff(u32KeyDownTime, g_u32SysTickCnt) > 2000)
				{
					ChangeAllLedState(false);						
					u32KeyDownTime = g_u32SysTickCnt;
					while (SysTimeDiff(u32KeyDownTime, g_u32SysTickCnt) < 2000);
				
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | 
						RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | 
						RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
						RCC_APB2Periph_AFIO, DISABLE);
					__disable_irq();
					
					/* disable after reset */
					PWR_WakeUpPinCmd(ENABLE);
					
					/* Request to enter STANDBY mode (Wake Up flag is cleared in PWR_EnterSTANDBYMode function) */
					PWR_EnterSTANDBYMode();
				}
				continue;
			}

			default:
				continue;
		}

		
		YNAGetCheckSum(pBuf);
		CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);	
	}
	return true;
}

static bool RockProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	u16 x, y, z;
	
#if SCENCE_MUTUAL
	TurnOffZoomScence();
	PresetNumInit();
#endif
	if (!g_boIsRockEnable)
	{
		return false;
	}

	pBuf = u8YNABuf;

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	pBuf[_YNA_Cmd] = pKeyIn->unKeyMixIn.stRockState.u8RockDir;
	x = pBuf[_YNA_Data1] = pKeyIn->unKeyMixIn.stRockState.u16RockXValue >> 1;
	y = pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.stRockState.u16RockYValue >> 1;

	z = pBuf[_YNA_Data3] = pKeyIn->unKeyMixIn.stRockState.u16RockZValue >> 3;
	if (g_boIsRockAus)
	{
		pBuf[_YNA_Data1] |= (0x01 << 6);
	}
	YNAGetCheckSum(pBuf);
	//CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);

#if 0	
	if (!g_boUsingUART2)
	{
		return true;
	}
#endif
	if (g_emProtocol == _Protocol_PecloD)
	{
		u8 u8Buf[7];
		u8Buf[_PELCOD_Sync] = 0xFF;
		u8Buf[_PELCOD_Addr] = g_u8CamAddr + 1;
		u8Buf[_PELCOD_Cmd1] = 0;
		u8Buf[_PELCOD_Cmd2] = pKeyIn->unKeyMixIn.stRockState.u8RockDir << 1;
		u8Buf[_PELCOD_Data1] = x;
		u8Buf[_PELCOD_Data2] = y;
		PelcoDGetCheckSum(u8Buf);
		CopyToUart2Message(u8Buf, 7);
		CopyToUartMessage(u8Buf, 7);
	}
	else
	{
		u8 u8Buf[16];
		u8 u8Cmd = pKeyIn->unKeyMixIn.stRockState.u8RockDir << 1;
		static bool boViscaNeedSendZoomStopCmd = false;
		static bool boViscaNeedSendDirStopCmd = false;
		static u8 u8Priority = 0;
		
		u8Cmd &= (PELCOD_DOWN | PELCOD_UP | 
					PELCOD_LEFT | PELCOD_RIGHT |
					PELCOD_ZOOM_TELE | PELCOD_ZOOM_WIDE);
		
		u8Buf[0] = 0x80 + g_u8CamAddr + 1;
		if (u8Priority == 0)
		{
			if ((u8Cmd & (PELCOD_DOWN | PELCOD_UP | PELCOD_LEFT | PELCOD_RIGHT)) != 0)
			{
				u8Priority = 1;
			}
			else if ((u8Cmd & (PELCOD_ZOOM_TELE | PELCOD_ZOOM_WIDE)) != 0)
			{
				u8Priority = 2;
			}
		}
		
		if (u8Priority == 1)
		{
			if (boViscaNeedSendDirStopCmd && 
				((u8Cmd & (PELCOD_DOWN | PELCOD_UP | PELCOD_LEFT | PELCOD_RIGHT)) == 0))
			{
				/* 81 01 06 01 18 18 03 03 FF */
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x06;
				u8Buf[3] = 0x01;
				u8Buf[4] = 0x00;
				u8Buf[5] = 0x00;
				u8Buf[6] = 0x03;
				u8Buf[7] = 0x03;
				u8Buf[8] = 0xFF;
				CopyToUart2Message(u8Buf, 9);
				CopyToUartMessage(u8Buf, 9);
				
				boViscaNeedSendDirStopCmd = false;
				if ((u8Cmd & (PELCOD_ZOOM_WIDE | PELCOD_ZOOM_WIDE)) != 0)
				{
					u8Priority = 2;
				}
				else
				{
					u8Priority = 0;					
				}
			}
			else
			{
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x06;
				u8Buf[3] = 0x01;
				if ((u8Cmd & (PELCOD_LEFT | PELCOD_RIGHT)) != 0)
				{
					u32 u32Tmp = 0x17 * x;
					u32Tmp /= 0x3F;
					u32Tmp %= 0x18;
					u32Tmp += 1;

					u8Buf[4] = u32Tmp;
					if ((u8Cmd & PELCOD_LEFT) != 0)
					{
						u8Buf[6] = 0x01;
					}
					else
					{
						u8Buf[6] = 0x02;
					}

				}
				else
				{
					u8Buf[4] = 0;
					u8Buf[6] = 0x03;
				}
				
				if ((u8Cmd & (PELCOD_UP | PELCOD_DOWN)) != 0)
				{
					u32 u32Tmp = 0x13 * y;
					u32Tmp /= 0x3F;
					u32Tmp %= 0x14;
					u32Tmp += 1;

					u8Buf[5] = u32Tmp;
					if ((u8Cmd & PELCOD_UP) != 0)
					{
						u8Buf[7] = 0x01;
					}
					else
					{
						u8Buf[7] = 0x02;
					}

				}
				else
				{
					u8Buf[5] = 0;
					u8Buf[7] = 0x03;
				}
				u8Buf[8] = 0xFF;
				CopyToUart2Message(u8Buf, 9);	
				CopyToUartMessage(u8Buf, 9);
				
				boViscaNeedSendDirStopCmd = true;			
			}	
		}
		
		if (u8Priority == 2)
		{
			if (boViscaNeedSendZoomStopCmd && 
					((u8Cmd & (PELCOD_ZOOM_WIDE | PELCOD_ZOOM_TELE)) == 0))
			{
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = 0x00;
				u8Buf[5] = 0xFF;
				CopyToUart2Message(u8Buf, 6);
				CopyToUartMessage(u8Buf, 6);
				
				boViscaNeedSendZoomStopCmd = false;
				u8Priority = 0;
			}
			else if ((u8Cmd & PELCOD_ZOOM_WIDE) == PELCOD_ZOOM_WIDE)
			{
				u32 u32Tmp = 0x05 * z;
				u32Tmp /= 0x0F;
				u32Tmp %= 6;
				u32Tmp += 2;
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = u32Tmp + 0x30;
				u8Buf[5] = 0xFF;
				CopyToUart2Message(u8Buf, 6);
				CopyToUartMessage(u8Buf, 6);

				boViscaNeedSendZoomStopCmd = true;

			}
			else
			{
				u32 u32Tmp = 0x05 * z;
				u32Tmp /= 0x0F;
				u32Tmp %= 6;
				u32Tmp += 2;
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = 0x20 + u32Tmp;
				u8Buf[5] = 0xFF;
				CopyToUart2Message(u8Buf, 6);			
				CopyToUartMessage(u8Buf, 6);			
				boViscaNeedSendZoomStopCmd = true;
			}
			
		}
		
		if (u8Cmd == 0)
		{
			u8Priority = 0;
		}
	}
	return true;
}
static bool PushPodProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	pBuf = u8YNABuf;
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	pBuf[_YNA_Cmd] = 0x80;
	
	pBuf[_YNA_Data1] |= 0x80;

	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.u32PushRodValue;
	YNAGetCheckSum(pBuf);
	CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
	
	{
		u32 u32Value = 	pBuf[_YNA_Data2];
		u32Value &= 0xFF;	

		if (u32Value > ROCK_MAX_VALUE  * 2 / 3)
		{
			ChangeLedState(GET_XY(_T_PushPod2), true);
			ChangeLedState(GET_XY(_T_PushPod1), false);
		}
		else if (u32Value < ROCK_MAX_VALUE / 3)
		{
			ChangeLedState(GET_XY(_T_PushPod2), false);
			ChangeLedState(GET_XY(_T_PushPod1), true);		
			
		}
		else
		{
			ChangeLedState(GET_XY(_T_PushPod2), true);
			ChangeLedState(GET_XY(_T_PushPod1), true);
		}

	}
	return true;
}


#if 0
static bool CodeSwitchProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	u16 u16Index;

	pBuf = u8YNABuf;
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	u16Index = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Index;
	switch (u16Index)
	{
		case 0x00:
		{
			pBuf[_YNA_Cmd] = 0x49;
			pBuf[_YNA_Data1] |= 0x80;
			break;
		}
		default:
			return false;

	}
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Cnt;
	YNAGetCheckSum(pBuf);
	CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
	return true;
	
}
#endif
static bool VolumeProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	pBuf = u8YNABuf;
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x06;

	pBuf[_YNA_Cmd] = 0x80;
	pBuf[_YNA_Data3] = pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.u32VolumeValue;
	YNAGetCheckSum(pBuf);
	CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
	return true;
}

static PFun_KeyProcess s_KeyProcessArr[_Key_Reserved] = 
{
	PushPodProcess, KeyBoardProcess, RockProcess,
	VolumeProcess, NULL, 
};


const u16 c_u16LedArrForMIDI[] = 
{
	_Led_QuickPlay1,
	_Led_QuickPlay2,
	_Led_QuickPlay3,
	_Led_QuickPlay4,
	
	_Led_Loop1,	
	_Led_Loop2,			
	_Led_Loop3,			
	_Led_Loop4,	

	_Led_FullScreen,
	_Led_MultiCorder,
	_Led_PlayList,
	_Led_StartExternal,
	_Led_Recording,
	_Led_Stream,
	
	
	/* ABUS */
	_Led_PGM_1,				
	_Led_PGM_2,
	_Led_PGM_3,
	_Led_PGM_4,
	_Led_PGM_5,
	_Led_PGM_6,
	_Led_PGM_7,
	_Led_PGM_8,
	_Led_PGM_9,
	_Led_PGM_10,
	_Led_PGM_11,
	_Led_PGM_12,		

	_Led_PVW_1,
	_Led_PVW_2,
	_Led_PVW_3,
	_Led_PVW_4,
	_Led_PVW_5,
	_Led_PVW_6,
	_Led_PVW_7,
	_Led_PVW_8,
	_Led_PVW_9,
	_Led_PVW_10,
	_Led_PVW_11,
	_Led_PVW_12,		

	_Led_Effect_Ctrl_Take,
	_Led_Effect_Ctrl_Cut,
	
	_Led_Overlay1,				
	_Led_Overlay2,
	_Led_Overlay3,
	_Led_Overlay4,
	_Led_Overlay5,
	_Led_Overlay6,
	_Led_Overlay7,
	_Led_Overlay8,
	_Led_Overlay9,
	_Led_Overlay10,
	_Led_Overlay11,
	_Led_Overlay12,		

	
	_Led_FTB,
	_Led_QuickPlay,


	_Led_Dsk1,
	_Led_Dsk2,
	_Led_Dsk3,
	_Led_Dsk4,

	_Led_Play1,
	_Led_Play2,
	_Led_Play3,
	_Led_Play4,

	_Led_Transition1,
	_Led_Transition2,
	_Led_Transition3,
	_Led_Transition4,
};

#define MIDI_KEY_BEGIN		0x20


static bool KeyBoardProcessForMIDI(StKeyMixIn *pKeyIn)
{
	u32 i;

	for (i = 0; i < pKeyIn->u32Cnt; i++)
	{
		StKeyState *pKeyState = pKeyIn->unKeyMixIn.stKeyState + i;
		
		if ((pKeyState->u8KeyValue >= _Key_Ctrl_Reserved_Inner1) ||
			(pKeyState->u8KeyValue < _Key_Ctrl_Begin))
		{
			continue;			
		}
		
		if (pKeyState->u8KeyState == KEY_KEEP)
		{
			continue;
		}
		else
		{
			u8 u8Midi[4] = {0x09, 0x90, 0x00, 0x7F};
			if (pKeyState->u8KeyState == KEY_UP)
			{
				u8Midi[0] = 0x08; 
				u8Midi[1] = 0x80; 
				u8Midi[3] = 0;			
			}

			u8Midi[2] = pKeyState->u8KeyValue - _Key_Ctrl_Begin + MIDI_KEY_BEGIN;
			
			if (IsUSBDeviceConnect())
			{
				CopyToUSBMessage(u8Midi, 4, _IO_USB_ENDP1);	
			}				

		}

	}
	return true;
}

static bool RockProcessForMIDI(StKeyMixIn *pKeyIn)
{
	u8 u8JoyStickBuf[4] = {0};
	s8 *pBuf = (s8 *)(u8JoyStickBuf);
	s16 s16Tmp = 0;
	u8 u8Dir = 0;

	
	u8Dir = pKeyIn->unKeyMixIn.stRockState.u8RockDir;
	
	if ((u8Dir & _YNA_CAM_LEFT) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockXValue);
		s16Tmp = 0 - (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockXValue);

		pBuf[0] = (s8)s16Tmp;
	}
	else if ((u8Dir & _YNA_CAM_RIGHT) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockXValue);

		pBuf[0] = (s8)s16Tmp;	
	}

	if ((u8Dir & _YNA_CAM_DOWN) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockYValue);

		pBuf[1] = (s8)s16Tmp;
	}
	else if ((u8Dir & _YNA_CAM_UP) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockYValue);
		s16Tmp = 0 - (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockYValue);

		pBuf[1] = (s8)s16Tmp;	
	}

	if ((u8Dir & _YNA_CAM_WIDE) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockZValue);
		s16Tmp = 0 - (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockZValue);

		pBuf[2] = (s8)s16Tmp;
	}
	else if ((u8Dir & _YNA_CAM_TELE) != 0)
	{
		s16Tmp = (s16)(pKeyIn->unKeyMixIn.stRockState.u16RockZValue);

		pBuf[2] = (s8)s16Tmp;	
	}
			
	if (IsUSBDeviceConnect())
	{
		CopyToUSBMessage(u8JoyStickBuf, 3, _IO_USB_ENDP2);
	}				
	
	return true;
}

void SBSendVolumeCmd(u16 u16Value)
{
}
static bool VolumeProcessForMIDI(StKeyMixIn *pKeyIn)
{
	SBSendVolumeCmd(pKeyIn->unKeyMixIn.u32VolumeValue);
	return true;
}

static bool PushPodProcessForMIDI(StKeyMixIn *pKeyIn)
{
	u8 u8Midi[4] = {0x0B, 0xB0, 0x0E};
	u16 u16Value = pKeyIn->unKeyMixIn.u32PushRodValue;
	
	u16Value = u16Value * 0x7F / PUSH_ROD_MAX_VALUE;
	
	u8Midi[3] = u16Value;

	if (IsUSBDeviceConnect())
	{
		CopyToUSBMessage(u8Midi, 4, _IO_USB_ENDP1);	
	}				

	return true;
}

static PFun_KeyProcess s_KeyProcessForMIDIArr[_Key_Reserved] = 
{
	PushPodProcessForMIDI, KeyBoardProcessForMIDI, RockProcessForMIDI,
	VolumeProcessForMIDI, NULL, 
};

void FlushMsgForMIDI()
{
#if 0
	if (IsUSBDeviceConnect())
	{
		static u32 u32PrevSendTime = 0;
		u8 u8Buf[24] = {1, 0};
		if (s_boIsDataUpdateForMIDI)
		{
			memcpy(u8Buf + 1, s_u8AllBackupForMIDI, 19);
			CopyToUSBMessage(u8Buf, 20);

			s_boIsDataUpdateForMIDI = false;
			u32PrevSendTime = g_u32SysTickCnt;
		}
		else if (SysTimeDiff(u32PrevSendTime, g_u32SysTickCnt) > 10000)
		{
			memcpy(u8Buf + 1, s_u8AllBackupForMIDI, 19);
			CopyToUSBMessage(u8Buf, 20);
		
			u32PrevSendTime = g_u32SysTickCnt;
		}
	}
#else
	if (IsUSBDeviceConnect())
	{
		static u32 u32PrevSendTime = 0;
		
		static u16 u16Id = _IO_USB_ENDP1;
		if (SysTimeDiff(u32PrevSendTime, g_u32SysTickCnt) > 200)
		{
			if (u16Id == _IO_USB_ENDP1)
			{
				static u8 u8Buf[8] = {0x09, 0x90, 60, 0x00};
				
				if (u8Buf[3] == 0)
				{
					u8Buf[3] = 0x7F;
					u8Buf[2]++;
					if (u8Buf[2] > 100)
					{
						u8Buf[2] = 20;
					}
				}
				else
				{
					u8Buf[3] = 0;
				}
				
				CopyToUSBMessage(u8Buf, 4, _IO_USB_ENDP1);			
				
				u16Id = _IO_USB_ENDP2;
			}
			else
			{
				static u8 u8Buf[8] = {0};
				u8Buf[0] = rand();
				u8Buf[1] = rand();
				u8Buf[2] = rand();
				CopyToUSBMessage(u8Buf, 3, _IO_USB_ENDP2);			
			
				u16Id = _IO_USB_ENDP1;
			}
			u32PrevSendTime = g_u32SysTickCnt;
		}
	}

#endif
}


bool KeyProcess(StIOFIFO *pFIFO)
{
	StKeyMixIn *pKeyIn = pFIFO->pData;
	
	if (pKeyIn->emKeyType >= _Key_Reserved)
	{
		return false;
	}
	if (s_KeyProcessArr[pKeyIn->emKeyType] != NULL)
	{
		s_KeyProcessArr[pKeyIn->emKeyType](pKeyIn);	
	}
	
	if (s_KeyProcessForMIDIArr[pKeyIn->emKeyType] != NULL)
	{
		s_KeyProcessForMIDIArr[pKeyIn->emKeyType](pKeyIn);		
	}
	return false;
}


bool PCEchoProcessYNA(StIOFIFO *pFIFO)
{
	u8 *pMsg;
	u8 u8Cmd;
	u8 u8Led;
	bool boIsLight;

	if (pFIFO == NULL)
	{
		return -1;
	}
	pMsg = (u8 *)pFIFO->pData;	
	u8Cmd = pMsg[_YNA_Cmd];
	
	boIsLight = !pMsg[_YNA_Data3];

	if (pMsg[_YNA_Mix] == 0x06)
	{
		return true;
	}
	
	u8Led = pMsg[_YNA_Data2];

	switch (u8Cmd)
	{
		case 0x44:
		{
			break;
		}
		case 0x45:
		{
			break;
		}
		case 0x46:
		{
			break;
		}
		case 0x47:
		{	
			switch (u8Led)
			{
				case 0x50: case 0x51: case 0x52: 
				case 0x53: case 0x54: case 0x55: case 0x56: 				
				{
					const u16 u16Led[] = 
					{
						_Led_Recording, _Led_Stream, 0xFFFF,
						_Led_FullScreen, _Led_MultiCorder, _Led_PlayList, _Led_StartExternal
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x50]), boIsLight);						
					break;
				}
				case 0x60: case 0x61: case 0x62: case 0x63:
				{
					const u16 u16Led[] = 
					{
						_Led_QuickPlay1, _Led_QuickPlay2, _Led_QuickPlay3, _Led_QuickPlay4
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x60]), boIsLight);						
					break;
				}
				
				case 0x70: case 0x71: case 0x72: case 0x73:
				{
					const u16 u16Led[] = 
					{
						_Led_Loop1, _Led_Loop2, _Led_Loop3, _Led_Loop4
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x70]), boIsLight);						
					break;
				}
				case 0x80: case 0x81: case 0x82: case 0x83:
				{
					const u16 u16Led[] = 
					{
						_Led_Dsk1, _Led_Dsk2, _Led_Dsk3, _Led_Dsk4
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x80]), boIsLight);						
					break;
				}
				case 0x90: case 0x91: case 0x92: case 0x93:
				{
					const u16 u16Led[] = 
					{
						_Led_Play1, _Led_Play2, _Led_Play3, _Led_Play4
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x90]), boIsLight);						
					break;
				}
				
				default:
					break;
			}

		
			break;
		}
		case 0x48:
		{
			const u16 u16LedPGM[] = 
			{
				_Led_PGM_1, _Led_PGM_2, _Led_PGM_3, _Led_PGM_4,
				_Led_PGM_5, _Led_PGM_6, _Led_PGM_7, _Led_PGM_8,
				_Led_PGM_9, _Led_PGM_10, _Led_PGM_11, _Led_PGM_12,

			};
			const u16 u16LedPVW[] = 
			{
				_Led_PVW_1, _Led_PVW_2, _Led_PVW_3, _Led_PVW_4,
				_Led_PVW_5, _Led_PVW_6, _Led_PVW_7, _Led_PVW_8,
				_Led_PVW_9, _Led_PVW_10, _Led_PVW_11, _Led_PVW_12,
			};

			switch (u8Led)
			{
				case 0x00:
				{
					ChangeLedStateWithBackgroundLight(GET_XY(_Led_Effect_Ctrl_Take), boIsLight);						
					break;
				}
				case 0x01:
				{
					ChangeLedStateWithBackgroundLight(GET_XY(_Led_Effect_Ctrl_Cut), boIsLight);						
					break;
				}
				case 0x02: case 0x03: case 0x04: case 0x05:
				case 0x06: case 0x07:
				{
					ChangeLedArrayState(u16LedPGM, sizeof(u16LedPGM) / sizeof(u16), false);
					ChangeLedStateWithBackgroundLight(GET_XY(u16LedPGM[pMsg[_YNA_Data2] - 0x02]), boIsLight);
					if (pMsg[_YNA_Data1] == 1)
					{
						SetTallyPGM(pMsg[_YNA_Data2] - 0x02, boIsLight, true, true);
						
						{
							u32 i;
							for (i = 0; i < 8; i++)
							{
								ExternIOCtrl(i, Bit_RESET);
							}
						}
	
						
						ExternIOCtrl(pMsg[_YNA_Data2] - 0x02, boIsLight ? Bit_SET : Bit_RESET);

					}
					break;
				}
				case 0x08: case 0x09: case 0x0A: case 0x0B:
				case 0x0C: case 0x0D: 
				{
					ChangeLedArrayState(u16LedPVW, sizeof(u16LedPVW) / sizeof(u16), false);
					ChangeLedStateWithBackgroundLight(GET_XY(u16LedPVW[pMsg[_YNA_Data2] - 0x08]), boIsLight);
					
					if (pMsg[_YNA_Data1] == 1)
					{
						SetTallyPVW(pMsg[_YNA_Data2] - 0x08, boIsLight, true, true);				
						
						{
							u32 i;
							for (i = 0; i < 8; i++)
							{
								ExternIOCtrl(i + 8, Bit_RESET);
							}
						}

						ExternIOCtrl(pMsg[_YNA_Data2] - 0x08 + 8, boIsLight ? Bit_SET : Bit_RESET);
					}
					break;
				}
				case 0x0E: case 0x0F: case 0x10: case 0x11:
				case 0x12: case 0x13:
				{
					break;
				}
				case 0x40:
				{
					break;
				}					
					
				case 0x50: case 0x51: case 0x52: case 0x53:
				case 0x54: case 0x55: case 0x56: case 0x57: 
				case 0x58: case 0x59: case 0x5A: case 0x5B:
				{
					const u16 u16Led[] = 
					{
						_Led_Overlay1,				
						_Led_Overlay2,
						_Led_Overlay3,
						_Led_Overlay4,
						_Led_Overlay5,
						_Led_Overlay6,
						_Led_Overlay7,
						_Led_Overlay8,
						_Led_Overlay9,
						_Led_Overlay10,
						_Led_Overlay11,
						_Led_Overlay12,		
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[u8Led - 0x50]), boIsLight);
					break;
				}
				case 0x60: case 0x61: case 0x62: case 0x63:
				{
					const u16 u16Led[] = 
					{
						_Led_Transition1,
						_Led_Transition2,
						_Led_Transition3,
						_Led_Transition4,					
					};
					ChangeLedStateWithBackgroundLight(GET_XY(u16Led[pMsg[_YNA_Data2] - 0x60]), boIsLight);
					break;
				}
				
				case 0x80: case 0x81: case 0x82: case 0x83:
				case 0x84: case 0x85: case 0x86: case 0x87:
				{
					ChangeLedArrayState(u16LedPGM, sizeof(u16LedPGM) / sizeof(u16), false);
					ChangeLedStateWithBackgroundLight(GET_XY(u16LedPGM[pMsg[_YNA_Data2] - 0x80 + 6]), boIsLight);
					if (pMsg[_YNA_Data1] == 1)
					{
						if (u8Led <= 0x81)
						{
							SetTallyPGM(pMsg[_YNA_Data2] - 0x80 + 6, boIsLight, true, true);
							
							{
								u32 i;
								for (i = 0; i < 8; i++)
								{
									ExternIOCtrl(i, Bit_RESET);
								}
							}

							ExternIOCtrl(pMsg[_YNA_Data2] - 0x80 + 6, boIsLight ? Bit_SET : Bit_RESET);
						}
					}

					break;
				}
				case 0x90: case 0x91: case 0x92: case 0x93:
				case 0x94: case 0x95: case 0x96: case 0x97:
				{
					ChangeLedArrayState(u16LedPVW, sizeof(u16LedPVW) / sizeof(u16), false);
					ChangeLedStateWithBackgroundLight(GET_XY(u16LedPVW[pMsg[_YNA_Data2] - 0x90 + 6]), boIsLight);
					if (pMsg[_YNA_Data1] == 1)
					{
						if (u8Led <= 0x91)
						{
							
							{
								u32 i;
								for (i = 0; i < 8; i++)
								{
									ExternIOCtrl(i + 8, Bit_RESET);
								}
							}
							SetTallyPVW(pMsg[_YNA_Data2] - 0x90 + 6, boIsLight, true, true);
							ExternIOCtrl(pMsg[_YNA_Data2] - 0x90 + 6 + 8, boIsLight ? Bit_SET : Bit_RESET);
						}					
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case 0x49:
		{
			break;
		}
		case 0x4A:
		{
			switch (u8Led)
			{
				case 0x04: 
				{
					ChangeLedStateWithBackgroundLight(GET_XY(_Led_FTB), boIsLight);
					break;
				}
				case 0x07: 
				{
					break;
				}
				case 0x08: 
				{
					ChangeLedStateWithBackgroundLight(GET_XY(_Led_QuickPlay), boIsLight);
					break;
				}
				default:
					break;
			}
			break;
		}
		case 0x4B:
		{
			switch (u8Led)
			{
				case 0x20: case 0x21: case 0x22: case 0x23:
				case 0x24: case 0x25: case 0x26: case 0x27:
				{
					break;
				}
				default:
					break;
			}
			break;
		}
		case 0x4C:
		{
			switch (u8Led)
			{
				case 0x70: case 0x71: case 0x72: case 0x73:
				case 0x74: case 0x75: case 0x76: case 0x77:
				case 0x78: case 0x79: case 0x7A: 
				{
					break;
				}
			}
			break;
		}
		case 0x80:
		{
		
			break;
		}
		case 0xC0:
		{
			if (pMsg[_YNA_Data2] == 0x01)
			{
				switch (pMsg[_YNA_Data3])
				{
					case 0x00:
					{
						u8 *pBuf = u8YNABuf;
						if (pBuf == NULL)
						{
							return false;
						}

						memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

						pBuf[_YNA_Sync] = 0xAA;
						pBuf[_YNA_Addr] = g_u8CamAddr;
						pBuf[_YNA_Mix] = 0x07;
						pBuf[_YNA_Cmd] = 0xC0;
						pBuf[_YNA_Data2] = 0x01;
						YNAGetCheckSum(pBuf);
						CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
						break;
					}
					case 0x02:
					{
						ChangeAllLedState(false);
						/* maybe we need turn on some light */
						BackgroundLightEnable(true);

						GlobalStateInit();
						break;
					}
					case 0x03:
					{
						u8 *pBuf = u8YNABuf;
						if (pBuf == NULL)
						{
							return false;
						}

						memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

						pBuf[_YNA_Sync] = 0xAA;
						pBuf[_YNA_Addr] = g_u8CamAddr;
						pBuf[_YNA_Mix] = 0x07;
						pBuf[_YNA_Cmd] = 0x80;
						pBuf[_YNA_Data2] = PushRodGetCurValue();
						YNAGetCheckSum(pBuf);
						CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
						break;
					}
				
					default:
						return false;
				}
			}				
			break;
		}
		default :
			return false;
		
	}

	return true;
}

bool PCEchoProcessForMIDI(StIOFIFO *pFIFO)
{
	u8 *pMsg;
	u8 u8Cmd;
	u8 u8Key;
	bool boIsLightON = true;

	if (pFIFO == NULL)
	{
		return false;
	}

	pMsg = (u8 *)pFIFO->pData;
	u8Cmd = (pMsg[1] & 0xF0);

	if ( u8Cmd == 0x80)
	{
		boIsLightON = false;
	}
	else if ((u8Cmd == 0x90) || (u8Cmd == 0xB0))
	{
		if ((pMsg[3] & 0x7F) == 0)
		{
			boIsLightON = false;
		}
	}
	else
	{
		return false;
	}
		
	
	u8Key = pMsg[2];
	
	if ((u8Key < MIDI_KEY_BEGIN) || 
		(u8Key > (MIDI_KEY_BEGIN + (_Key_Ctrl_Reserved_Inner1 - _Key_Ctrl_Begin))))
	{
		return false;
	}

	u8Key = u8Key - MIDI_KEY_BEGIN + _Key_Ctrl_Begin;
	
	if ((u8Key >= _Key_Ctrl_Begin) && 
		(u8Key <_Key_Ctrl_Reserved_Inner1))
	{
		u16 u16Led = c_u16LedArrForMIDI[u8Key - _Key_Ctrl_Begin];
		ChangeLedStateWithBackgroundLight(GET_XY(u16Led), boIsLightON);
		
		if (u8Key >= _Key_PGM_1 && u8Key <= _Key_PGM_8)
		{
			u8Key -= _Key_PGM_1;
			ExternIOCtrl(u8Key, boIsLightON ? Bit_SET : Bit_RESET);
		}
		else if(u8Key >= _Key_PVW_1 && u8Key <= _Key_PVW_8)
		{
			u8Key -= _Key_PVW_1;
			ExternIOCtrl(u8Key + 8, boIsLightON ? Bit_SET : Bit_RESET);
		}
		
	}
	
	return true;
}

bool PCEchoProcessForHIDSB(StIOFIFO *pFIFO)
{
	u8 *pMsg;

	if (pFIFO == NULL)
	{
		return false;
	}
	pMsg = (u8 *)pFIFO->pData;	
	
	CopyToUart1Message(pMsg, MIDI_JACK_SIZE);

	return true;
	
}
bool PCEchoProcess(StIOFIFO *pFIFO)
{
	if (pFIFO->u8ProtocolType == _Protocol_YNA)
	{
		return PCEchoProcessYNA(pFIFO);
	}
	else if (pFIFO->u8ProtocolType == _Protocol_MIDI)
	{
		return PCEchoProcessForMIDI(pFIFO);
	}
	
	return false;
}

void BackgroundLightEnable(bool boIsEnable)
{
	if (g_boIsBackgroundLightEnable)
	{
		u32 i;		
		for (i = 0; i < sizeof(c_u16LedArrForMIDI) / sizeof(u16); i++)
		{
			u16 u16Led = c_u16LedArrForMIDI[i];
			if (u16Led == 0xFFFF)
			{
				continue;
			}
		
			{
				u16 x = GET_X(u16Led) + 1;
				u16 y = GET_Y(u16Led);
				
				ChangeLedState(x, y, boIsEnable);
			}
		}
	}
}

void ChangeLedStateWithBackgroundLight(u32 x, u32 y, bool boIsLight)
{
	if (g_boIsBackgroundLightEnable)
	{
		if (x != 0xFF)
		{
			ChangeLedState(x + 1, y, !boIsLight);
		}
	}

	ChangeLedState(x, y, boIsLight);
}


