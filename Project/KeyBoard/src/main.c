#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
#include "pwm.h"


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

#include "hw_config.h"
#include "usb_desc.h"
#include "usb_lib.h"
#include "usb_pwr.h"



int main()
{
	u32 u32SyncCount = 0;
	u32 u32RedressTime;

	{
		if (PWR_GetFlagStatus(PWR_FLAG_SB) == SET)
		{
			PWR_ClearFlag(PWR_FLAG_SB);
		}
		if (PWR_GetFlagStatus(PWR_FLAG_WU) == SET)
		{
			PWR_ClearFlag(PWR_FLAG_WU);
		}
		
		PWR_WakeUpPinCmd(DISABLE);

	}

	KeyBufInit();
	GlobalStateInit();
	
	PeripheralPinClkEnable();
	OpenSpecialGPIO();

	ReadSaveData();
	MessageUartInit();
	MessageUart3Init();

#if 1	
	KeyLedInit();
	ChangeAllLedState(false);
	RockPushRodInit();
	BuzzerInit();
	
	ExternIOInit();
	ExternIOClear();
	
	//CodeSwitchInit();
	//PWMCtrlInit();
#endif

	SysTickInit();
	ChangeEncodeState();

	/* must after SysTickInit */
	MessageUSBInit();


#if 1
	/* 打开所有LED */
	ChangeAllLedState(true);

	u32RedressTime = g_u32SysTickCnt;
	while(1)
	{
		void *pKeyIn = KeyBufGetBuf();	
		if (pKeyIn != NULL)
		{
			if (ProtocolSelect(pKeyIn))
			{
				u32RedressTime = g_u32SysTickCnt;
			}
			else if (RedressPushRodLimit(pKeyIn))
			{
				u32RedressTime = g_u32SysTickCnt;
			}
			else
			{
				KeyBufGetEnd(pKeyIn);				
			}		
		}
		ChangeAllLedState(true);
			
		if(SysTimeDiff(u32RedressTime, g_u32SysTickCnt) > 5000)
		{
			break;
		}

	}

	ChangeAllLedState(true);
	u32SyncCount = 0;
	while (u32SyncCount < 3)
	{
		u8 u8Buf[PROTOCOL_YNA_ENCODE_LENGTH];
		u8 *pBuf = NULL;
		void *pMsgIn = NULL; 

		u32RedressTime = g_u32SysTickCnt;

		pBuf = u8Buf;
		memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

		pBuf[_YNA_Sync] = 0xAA;
		pBuf[_YNA_Mix] = 0x07;
		pBuf[_YNA_Cmd] = 0xC0;

		YNAGetCheckSum(pBuf);
		CopyToUartMessage(pBuf, PROTOCOL_YNA_DECODE_LENGTH);
		
		pMsgIn = MessageUartFlush(false);
		if (pMsgIn != NULL)
		{
			MessageUartRelease(pMsgIn);
			break;
		}
		u32SyncCount++;		
		while(SysTimeDiff(u32RedressTime, g_u32SysTickCnt) < 1000);/* 延时1S */
	}
#endif	

	ChangeAllLedState(false);
	GlobalStateInit();
	BackgroundLightEnable(true);
	do 
	{
		void *pFIFO = KeyBufGetBuf();
		if (pFIFO == NULL)
		{
			break;
		}
		KeyBufGetEnd(pFIFO);
	}while(1);

	while (1)
	{
		void *pMsgIn = MessageUartFlush(false);
		void *pKeyIn = KeyBufGetBuf();
		void *pMsgIn3 = MessageUart3Flush(false);
		void *pMsgUSB = MessageUSBFlush(false);

		if (pKeyIn != NULL)
		{
			KeyProcess(pKeyIn);
		}	
		if (pMsgIn != NULL)
		{
			if (BaseCmdProcess(pMsgIn, &c_stUartIOTCB) != 0)			
			{
				PCEchoProcess(pMsgIn);
			}
		}
		
		if (pMsgUSB != NULL)
		{
#if 0
			StIOFIFO *pFIFO = pMsgUSB;
			if (pFIFO->s32Length <= REPORT_IN_SIZE)
			{
				u8 u8Buf[24] = {REPORT_ID, 0};
				memcpy(u8Buf + 1, pFIFO->pData, pFIFO->s32Length);
				CopyToUSBMessage(u8Buf, REPORT_IN_SIZE_WITH_ID);								
			}
#endif
			PCEchoProcess(pMsgUSB);
			
		}
		
		KeyBufGetEnd(pKeyIn);				
		MessageUartRelease(pMsgIn);	
		MessageUart3Release(pMsgIn3);	
		MessageUSBRelease(pMsgUSB);	
		
		//FlushMsgForMIDI();
	}
}
