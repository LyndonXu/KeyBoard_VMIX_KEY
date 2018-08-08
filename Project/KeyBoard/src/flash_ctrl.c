#include <stddef.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "common.h"
#include "user_api.h"

#include "adc_ctrl.h"
#include "pwm.h"

#include "code_switch.h"
#include "key_led_ctrl.h"


#include "protocol.h"
#include "flash_ctrl.h"


void LockApplication(bool boIsLock)
{
	FLASH_Status emFLASHStatus = FLASH_COMPLETE;
	uint32_t u32WRPRValue;
	uint32_t u32ProtectedPages;
	/* Unlock the Flash Program Erase controller */  
	FLASH_Unlock();

	(void)emFLASHStatus;
	
	/* Get pages write protection status */
	u32WRPRValue = FLASH_GetWriteProtectionOptionByte();

	if (boIsLock)
	{
	
		/* Get current write protected pages and the new pages to be protected */
		u32ProtectedPages =  (~u32WRPRValue) | FLASH_PAGES_TO_BE_PROTECTED; 

		/* Check if desired pages are not yet write protected */
		if(((~u32WRPRValue) & FLASH_PAGES_TO_BE_PROTECTED )!= FLASH_PAGES_TO_BE_PROTECTED)
		{
			/* Erase all the option Bytes because if a program operation is 
			performed on a protected page, the Flash memory returns a 
			protection error */
			emFLASHStatus = FLASH_EraseOptionBytes();

			/* Enable the pages write protection */
			emFLASHStatus = FLASH_EnableWriteProtection(u32ProtectedPages);

			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
	}
	else
	{
		/* Get pages already write protected */
		u32ProtectedPages = ~(u32WRPRValue | FLASH_PAGES_TO_BE_PROTECTED);

		/* Check if desired pages are already write protected */
		if((u32WRPRValue | (~FLASH_PAGES_TO_BE_PROTECTED)) != 0xFFFFFFFF )
		{
			/* Erase all the option Bytes */
			emFLASHStatus = FLASH_EraseOptionBytes();

			/* Check if there is write protected pages */
			if(u32ProtectedPages != 0x0)
			{
				/* Restore write protected pages */
				emFLASHStatus = FLASH_EnableWriteProtection(u32ProtectedPages);
			}
			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
	}
	FLASH_Lock();

}

int32_t FlashWritePage( uint32_t u32FlashStartAddress, void *pBuf, uint32_t u32Length)
{
	FLASH_Status emFLASHStatus = FLASH_COMPLETE;
	uint32_t u32NumOfPage;
	uint32_t i;
	uint32_t u32CurWriteAddress;
	uint32_t u32FlashEndAddress;
	uint32_t *pData = (uint32_t *)pBuf;

	if ((u32Length & (~0x00000003)) != u32Length)
	{
		return -1;
	}
#if 0
	u32Length /= FLASH_PAGE_SIZE;
	u32Length *= FLASH_PAGE_SIZE;
	if (u32Length < FLASH_PAGE_SIZE)
	{
		return -1; /* too small */
	}
#endif	
	u32FlashStartAddress /= FLASH_PAGE_SIZE;
	u32FlashStartAddress *= FLASH_PAGE_SIZE;
	u32FlashEndAddress = u32FlashStartAddress + u32Length;
	
	/* Unlock the Flash Program Erase controller */  
	FLASH_Unlock();
	/* Get the number of pages to be erased */
	u32NumOfPage = (u32Length + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE;
	

	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);	

	/* erase the FLASH pages */
	for(i = 0; (i < u32NumOfPage) && (emFLASHStatus == FLASH_COMPLETE); i++)
	{
		emFLASHStatus = FLASH_ErasePage(u32FlashStartAddress + (FLASH_PAGE_SIZE * i));
	}
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		goto end;
	}
	/* FLASH Half Word program of data 0x1753 at addresses defined by  BANK1_WRITE_START_ADDR and BANK1_WRITE_END_ADDR */
	u32CurWriteAddress = u32FlashStartAddress;

	i = 0;
	while((u32CurWriteAddress < u32FlashEndAddress) && (emFLASHStatus == FLASH_COMPLETE))
	{
		emFLASHStatus = FLASH_ProgramWord(u32CurWriteAddress, pData[i++]);
		u32CurWriteAddress = u32CurWriteAddress + 4;
	}
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		goto end;
	}

	/* Check the correctness of written data */
	u32CurWriteAddress = u32FlashStartAddress;
	emFLASHStatus = FLASH_COMPLETE;
	i = 0;
	while((u32CurWriteAddress < u32FlashEndAddress) && (emFLASHStatus != FLASH_COMPLETE))
	{
		if((*(__IO uint32_t*) u32CurWriteAddress) != pData[i++])
		{
			emFLASHStatus = FLASH_ERROR_PG;
		}
		u32CurWriteAddress += 4;
	}
	FLASH_Unlock();

#if 0	
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_EraseOptionBytes();
	FLASH_ProgramOptionByteData(0x1FFFF804,0xB0);
#endif

end:
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		return -1;
	}
	return 0;
}


void GetUID(StUID *pUID)
{
	if (pUID != NULL)
	{
		//*pUID = *((StUID *)UID_BASE_ADDR);
		int32_t s32Length = sizeof(StUID);
		memcpy(pUID, (void *)UID_BASE_ADDR, s32Length);
	}
}


uint32_t AppCRC32(uint32_t u32AppSize)
{
	uint32_t u32CheckSize = 0;
	uint32_t *pAppData = (uint32_t *)APP_START_ADDRESS;
	uint32_t i = 0;
	while (u32CheckSize < u32AppSize)
	{
		if(pAppData[i] == 0xFFFFFFFF)
		{
			if ((pAppData[i + 1] == ~0) 
				&& (pAppData[i + 2] == ~0)
				&& (pAppData[i + 3] == ~0))
			{
				break;
			}
		}
		u32CheckSize += 4;
		i++;
	}
	
	return CRC32Buf((uint8_t *)APP_START_ADDRESS, u32CheckSize);
}

int32_t GetLic(StBteaKey *pKey, StUID *pUID, uint32_t u32AppCRC32, bool boIsRelease)
{
	if((pKey == NULL) || (pUID == NULL))
	{
		return -1;
	}
	else
	{
		StBteaKey stKey;
		
		stKey.stLIC.stUID = *pUID;
		stKey.stLIC.u32CRC32 = u32AppCRC32;
		if (boIsRelease)
		{
			pKey->stLIC = stKey.stLIC;
			
			btea(pKey->s32ReleaseLIC, 4, stKey.s32Key);
		}
		else
		{
			pKey->s32RCLic = LIC_RC_HEADER;
			
			btea(&pKey->s32RCLic, 1, stKey.s32Key);
		}
		
		return 0;
	}
}

int32_t WriteLic(StBteaKey *pKey, bool boIsRelease, uint32_t u32RCCount)
{
	if (pKey == NULL)
	{
		return -1;
	}
	else
	{
		FLASH_Status emFLASHStatus = FLASH_COMPLETE;

		uint32_t i;
		StLICCtrl stCtrl;
		uint32_t *pTmp = (uint32_t *)(&stCtrl);
		
		stCtrl.stBteaKey = *pKey;
		if (boIsRelease)
		{
			stCtrl.u32Header = LIC_RELEASE_HEADER;
		}
		else
		{
			stCtrl.u32Header = LIC_RC_HEADER;
			/* xxtea the counter */
			stCtrl.u64RCCount = u32RCCount;
			btea((int32_t *)(&stCtrl.u64RCCount), 2, stCtrl.stBteaKey.s32Key);
		}
		stCtrl.u32CheckSum = 0;
		for(i = 0; i < offsetof(StLICCtrl, u32CheckSum) / sizeof(uint32_t); i++)
		{
			stCtrl.u32CheckSum ^= pTmp[i];
		}
		
		FLASH_Unlock();
		
		/* Clear All pending flags */
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
		
		emFLASHStatus = FLASH_ErasePage(LIC_ADDRESS);

		pTmp = (uint32_t *)(&stCtrl);
		for (i = 0; i < (sizeof(StLICCtrl) / sizeof(uint32_t)); i++)
		{
			emFLASHStatus = FLASH_ProgramWord(LIC_ADDRESS + i * 4, pTmp[i]);
			if (emFLASHStatus != FLASH_COMPLETE)
			{
				FLASH_Lock();
				return -1;
			}
		}
		
		FLASH_Lock();
		return 0;
	}
}

void CheckLIC(PFUN_CheckLicCB pFunCB, void *pContext)
{
	static EmLicStatus emStatus = _LicStatus_StartUp;
	static uint32_t u32CheckTime = 0;
	static StUID stUID;
	static uint32_t u32CRC32 = ~0;
	static StBteaKey stLicRelease;
	static StBteaKey stLicRC;
	
	StLICCtrl *pCtrl = (StLICCtrl *)LIC_ADDRESS;
	
	if (u32CRC32 == ~0)
	{
		GetUID(&stUID);
		u32CRC32 = AppCRC32(~0);
		GetLic(&stLicRC, &stUID, u32CRC32, false);
		GetLic(&stLicRelease, &stUID, u32CRC32, true);
	}
	//WriteLic(&stLicRC, false, 0);
	
	if (emStatus == _LicStatus_StartUp) /* first startup */
	{
		if ((pCtrl->u32Header == LIC_RC_HEADER) 
			|| (pCtrl->u32Header == LIC_RELEASE_HEADER))
		{
			uint32_t *pTmp = (uint32_t *)LIC_ADDRESS;
			uint32_t u32CheckSum = 0;
			uint32_t i;
			for(i = 0; i < offsetof(StLICCtrl, u32CheckSum) / sizeof(uint32_t); i++)
			{
				u32CheckSum ^= pTmp[i];
			}
			if (u32CheckSum == pCtrl->u32CheckSum)
			{

				if (pCtrl->u32Header == LIC_RC_HEADER)
				{
					/* valid */
					if (stLicRC.s32RCLic == pCtrl->stBteaKey.s32RCLic)
					{
						uint64_t u64RCCount = pCtrl->u64RCCount;
						btea((int32_t *)(&u64RCCount), -2, pCtrl->stBteaKey.s32Key);
						if (u64RCCount > LIC_RC_MAX_COUNT)
						{
							emStatus = _LicStatus_InValid;							
						}
						else
						{	
							StBteaKey stLicTmp = stLicRC;
							WriteLic(&stLicTmp, false, u64RCCount + 1);
							emStatus = _LicStatus_RC;
						}
					}
					else		/* has been modified spitefully */
					{
						StBteaKey stLicTmp = stLicRC;
						WriteLic(&stLicTmp, false, LIC_RC_MAX_COUNT + 1);
						emStatus = _LicStatus_InValid;
					}						
				}
				else
				{
					/* valid */
					if (memcmp(&stLicRelease, &pCtrl->stBteaKey, sizeof(StBteaKey)) == 0)
					{
						emStatus = _LicStatus_Release;
					}
					else/* has been modified spitefully */
					{
						StBteaKey stLicTmp = stLicRC;
						WriteLic(&stLicTmp, false, LIC_RC_MAX_COUNT + 1);
						emStatus = _LicStatus_InValid;
					}			
				}
			}
		}
		else	/* write  the RC LIC */
		{
			StBteaKey stLicTmp = stLicRC;
			WriteLic(&stLicTmp, false, 0);
			emStatus = _LicStatus_RC;
		}
	}
	if (emStatus == _LicStatus_RC)
	{
		if (SysTimeDiff(u32CheckTime, g_u32SysTickCnt) > LIC_RC_TIMEOUT)
		{
			uint64_t u64RCCount = pCtrl->u64RCCount;
			StBteaKey stLicTmp = pCtrl->stBteaKey;
			
			u32CheckTime = g_u32SysTickCnt; 
			
			btea((int32_t *)(&u64RCCount), -2, pCtrl->stBteaKey.s32Key);
			WriteLic(&stLicTmp, false, u64RCCount + 1);
			if (u64RCCount > LIC_RC_MAX_COUNT)
			{
				emStatus = _LicStatus_InValid;
			}
			
			/* call the callback */
			if (pFunCB != NULL)
			{
				pFunCB(_LicStatus_RC_Timeout, pContext);
			}
		}
	}
	
	if (emStatus == _LicStatus_Release)
	{
	
	}
	
	if (emStatus == _LicStatus_InValid)
	{
		if (memcmp(&stLicRelease, &pCtrl->stBteaKey, sizeof(StBteaKey)) == 0)
		{
			emStatus = _LicStatus_Release;
			return;
		}
		/* call the callback */
		if (pFunCB != NULL)
		{
			pFunCB(_LicStatus_InValid, pContext);
		}
		
		if (SysTimeDiff(u32CheckTime, g_u32SysTickCnt) > LIC_INVALID_TIMEOUT)
		{
			u32CheckTime = g_u32SysTickCnt; 
			/* call the callback */
			if (pFunCB != NULL)
			{
				pFunCB(_LicStatus_InValid_Timeout, pContext);
			}
		}
	}
}

int32_t SetOptionByte(uint8_t u8Data)
{
	FLASH_Status emStatus = FLASH_COMPLETE;
	FLASH_Unlock();		
	do
	{
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		emStatus = FLASH_EraseOptionBytes();
		if (emStatus != FLASH_COMPLETE)
		{
			break;
		}
		emStatus = FLASH_ProgramOptionByteData(OPTION_BYTE_ADDRESS, u8Data);
	
	} while (0);
	FLASH_Lock();

	return 	emStatus == FLASH_COMPLETE ? 0 : -1;
}

uint8_t GetOptionByte(void)
{
	return *((uint8_t *)OPTION_BYTE_ADDRESS);
}


typedef struct _tagStSave
{
	u16 u16Head;
	u16 u16Times;
	u16 u16UpLimit;
	u16 u16DownLimit;
	u16 u16VolumeTimes;
	u16 u16VolumeUpLimit;
	u16 u16VolumeDownLimit;
	u16 u16Protocol;
	u16 u16CheckDum;
}StSave;

#define CHECK_SIZE		((sizeof(StSave) / sizeof(u16)) - 1)

void ReadSaveData(void)
{
	StSave stSave = {0};

	u16 u16CheckSum = 0, *pTmp = (u16 *)(&stSave);
	u32 i;
	
	memcpy(&stSave, (void *)DATA_SAVE_ADDR, sizeof(StSave));

	if (stSave.u16Head != DATA_SAVE_HEAD)
	{
		goto end;
	}

	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pTmp[i];
	}

	if (u16CheckSum == stSave.u16CheckDum)
	{	
		g_u16Times = stSave.u16Times;
		g_u16UpLimit = stSave.u16UpLimit;
		g_u16DownLimit = stSave.u16DownLimit;
		
		g_u16VolumeTimes = stSave.u16VolumeTimes;
		g_u16VolumeUpLimit = stSave.u16VolumeUpLimit;
		g_u16VolumeDownLimit = stSave.u16VolumeDownLimit;
		

		g_emProtocol = (EmProtocol)stSave.u16Protocol;
		return;
	}
	
end:

	g_u16Times = PUSH_ROD_TIMES;
	g_u16UpLimit = PUSH_ROD_END;
	g_u16DownLimit = PUSH_ROD_BEGIN;
	
	g_u16VolumeTimes = VOLUME_TIMES;
	g_u16VolumeUpLimit = VOLUME_END;
	g_u16VolumeDownLimit = VOLUME_BEGIN;
	

	g_emProtocol = _Protocol_YNA;
	
}
bool WriteSaveData(void)
{
	FLASH_Status FLASHStatus = FLASH_COMPLETE;

	StSave stSave = {0};

	u16 u16CheckSum = 0, *pData = (u16 *)(&stSave);
	u32 i, u32Addr = DATA_SAVE_ADDR;
	
	stSave.u16Head = DATA_SAVE_HEAD;
	
	stSave.u16Times = g_u16Times;
	stSave.u16UpLimit = g_u16UpLimit;
	stSave.u16DownLimit = g_u16DownLimit;
	
	stSave.u16VolumeTimes = g_u16VolumeTimes;
	stSave.u16VolumeUpLimit = g_u16VolumeUpLimit;
	stSave.u16VolumeDownLimit = g_u16VolumeDownLimit;
	
	stSave.u16Protocol = g_emProtocol;

	
	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pData[i];
	}
	stSave.u16CheckDum = u16CheckSum;


	FLASH_Unlock();
		
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	
	FLASHStatus = FLASH_ErasePage(DATA_SAVE_ADDR);

	
	for (i = 0; i < (CHECK_SIZE + 1); i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(u32Addr, pData[i]);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
	}
	
	FLASH_Lock();
	return true;
}


s32 Compare(const void *pLeft, const void *pRight)
{
	return (*((u16 *)pLeft)) - (*((u16 *)pRight));
}

#if 0
bool CheckUID(const StUID *pUID)
{
	u32 i;
	u16 *pData;
	u16 u16ORNum;

	pData = (u16 *)UID_CHECK_ADDR;

	u16ORNum = *pData;

	for (i = 1; i < GET_UID_CNT(sizeof(u16)) + 1; i++)
	{
		u16 u16TmpUID;
		if (pData[i] > 0x3FF)
		{
			return false;
		}
		u16TmpUID = pData[pData[i]];
		u16TmpUID ^= u16ORNum;
		if (u16TmpUID != pUID->u16UID[i - 1])
		{
			return false;
		}
		
	}
	return true;
}

bool WriteUID(const StUID *pUID, u32 u32Srand)
{
	u32 i;
	u32 u32Addr = UID_CHECK_ADDR;
	u16 *pData;
	u16 u16ORNum;
	StUID stUIDRand = {0};
	StUID stUIDAddr = {0};
	
	u16 u16Cnt = 0;

	FLASH_Status FLASHStatus = FLASH_COMPLETE;
	
	
	srand(u32Srand);
	u16ORNum = rand();
	for (i = 0; i < GET_UID_CNT(sizeof(u16)); i++)
	{
		u32 j;
		stUIDRand.u16UID[i] = pUID->u16UID[i] ^ u16ORNum;
		while(1)
		{
			bool boIsGetAGoodAddr = true;
			stUIDAddr.u16UID[i] = rand() & 0x3FF;
			if (stUIDAddr.u16UID[i] <= (GET_UID_CNT(sizeof(u16)) + 1))
			{
				continue;
			}
			for (j = 0; j < i; j++)
			{
				if (stUIDAddr.u16UID[j] == stUIDAddr.u16UID[i])
				{
					boIsGetAGoodAddr = false;
					break;
				}
			}
			if (boIsGetAGoodAddr)
			{
				break;
			}
		}
		
	}
	pData = stUIDAddr.u16UID;
	qsort(pData, GET_UID_CNT(sizeof(u16)), sizeof(u16), Compare);

#if 1
	FLASH_Unlock();
		
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	
	FLASHStatus = FLASH_ErasePage(u32Addr);
	FLASHStatus = FLASH_ErasePage(u32Addr + 1024);

	FLASH_ProgramHalfWord(u32Addr, u16ORNum);
	u32Addr += sizeof(u16);
	
	for (i = 1; i < GET_UID_CNT(sizeof(u16)) + 1; i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(u32Addr, pData[i - 1]);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
		
	}
	
	for (; i < 1024; i++)
	{
		if (i == stUIDAddr.u16UID[u16Cnt])
		{
			FLASHStatus = FLASH_ProgramHalfWord(u32Addr, stUIDRand.u16UID[u16Cnt++]);
		}
		else
		{
			FLASHStatus = FLASH_ProgramHalfWord(u32Addr, rand());
		}
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
	}
	
	FLASH_Lock();
#endif
	return true;
}
#endif
