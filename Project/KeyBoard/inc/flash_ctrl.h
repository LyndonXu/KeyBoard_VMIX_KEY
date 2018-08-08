#ifndef _FLASH_CTRL_H_
#define _FLASH_CTRL_H_
#include "stdint.h"
#include "stdbool.h"

#include "stm32f10x_conf.h"
#include "common.h"
#include "user_api.h"

//#include "audio_volume.h"
//#include "audio_mode.h"
//#include "audio_out.h"
//#include "fantasy_power.h"

/* Define the STM32F10x FLASH Page Size depending on the used device */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
  #define FLASH_PAGES_TO_BE_PROTECTED (FLASH_WRProt_Pages12to13 | FLASH_WRProt_Pages14to15)  
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
  #define FLASH_PAGES_TO_BE_PROTECTED ( \
			FLASH_WRProt_Pages12to15 | FLASH_WRProt_Pages16to19 \
			| FLASH_WRProt_Pages20to23 | FLASH_WRProt_Pages24to27 \
			| FLASH_WRProt_Pages28to31 | FLASH_WRProt_Pages32to35 \
			| FLASH_WRProt_Pages36to39 | FLASH_WRProt_Pages40to43 \
			| FLASH_WRProt_Pages44to47 | FLASH_WRProt_Pages48to51 \
			| FLASH_WRProt_Pages52to55 | FLASH_WRProt_Pages56to59 \
			| FLASH_WRProt_Pages60to63 | FLASH_WRProt_Pages64to67 \
			| FLASH_WRProt_Pages68to71 | FLASH_WRProt_Pages72to75 \
			| FLASH_WRProt_Pages76to79 | FLASH_WRProt_Pages80to83 \
			| FLASH_WRProt_Pages84to87 | FLASH_WRProt_Pages88to91 \
			| FLASH_WRProt_Pages92to95 | FLASH_WRProt_Pages96to99 \
			)  
#endif

#define OPTION_BYTE_ADDRESS			0x1FFFF804
#define OPTION_UPGRADE_DATA			0xB0
#define OPTION_NORMAL_DATA			0xFF

#define BOOTLOADER_START_ADDRESS	0x08000000
#define BOOTLOADER_END_ADDRESS		(0x08003000 - 1)

#define APP_START_ADDRESS			0x08003000
#define APP_END_ADDRESS				(0x08019000 - 1)

#define LIC_ADDRESS					0x08019000

#define EEPROM_ADDRESS				(LIC_ADDRESS + FLASH_PAGE_SIZE)
#define EEPROM_DATA_HEAD  			(0xA5A5)


#define GET_UID_CNT(Byte)			(96 / (Byte * 8))
#define UID_BASE_ADDR				(0x1FFFF7E8)




#pragma anon_unions
typedef struct _tagStUID
{
	union
	{
		uint16_t u16UID[GET_UID_CNT(sizeof(uint16_t))];
		uint32_t u32UID[GET_UID_CNT(sizeof(uint32_t))];
		uint8_t u8UID[GET_UID_CNT(sizeof(uint8_t))];
	};
}StUID;

typedef struct _tagStLIC
{
	StUID stUID;
	uint32_t u32CRC32;
}StLIC;

typedef struct _tagStBteaKey
{
	union
	{
		StLIC stLIC;
		int32_t s32Key[4];
		int32_t s32ReleaseLIC[4];
		int32_t s32RCLic;
	};
}StBteaKey;

typedef struct _taStLICCtrl
{
	uint32_t u32Header;
	StBteaKey stBteaKey;
	uint64_t u64RCCount;
	uint32_t u32CheckSum;
}StLICCtrl;

#define LIC_RC_HEADER			0x01234567
#define LIC_RELEASE_HEADER		0x89ABCDEF

#define LIC_RC_MAX_COUNT		3
#define LIC_RC_TIMEOUT			(20 * 1000)
#define LIC_INVALID_TIMEOUT		(20 * 1000)

typedef enum _tagEmLicStatus
{
	_LicStatus_StartUp,
	_LicStatus_RC,
	_LicStatus_RC_Timeout,
	_LicStatus_Release,
	_LicStatus_InValid,
	_LicStatus_InValid_Timeout,
}EmLicStatus;


void LockApplication(bool boIsLock);
int32_t FlashWritePage( uint32_t u32FlashStartAddress, 
	void *pBuf, uint32_t u32Length);
void GetUID(StUID *pUID);
uint32_t AppCRC32(uint32_t u32AppSize);
int32_t GetLic(StBteaKey *pKey, StUID *pUID, uint32_t u32AppCRC32, bool boIsRelease);
int32_t WriteLic(StBteaKey *pKey, bool boIsRelease, uint32_t u32RCCount);
typedef int32_t (*PFUN_CheckLicCB)(EmLicStatus emStatus, void *pContext);
void CheckLIC(PFUN_CheckLicCB, void *pContext);
int32_t SetOptionByte(uint8_t u8Data);
uint8_t GetOptionByte(void);



bool WriteSaveData(void);
void ReadSaveData(void);

	
#endif
