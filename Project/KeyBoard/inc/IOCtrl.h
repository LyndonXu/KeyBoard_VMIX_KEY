#ifndef IOCTRL_H_
#define IOCTRL_H_
#include <stdint.h>
#include <stdbool.h>


typedef struct _tagStIOFIFOList StIOFIFOList;

typedef struct _tagStIOFIFO
{
	void *pData;
	int32_t s32Length;
	bool boNeedFree;
	uint8_t u8DataType;
	uint8_t u8ProtocolType;
	uint16_t u16ID;
}StIOFIFO;

struct _tagStIOFIFOList
{
	void *pData;
	int32_t s32Length;
	bool boNeedFree;
	uint8_t u8DataType;
	uint8_t u8ProtocolType;
	uint16_t u16ID;
	
	StIOFIFOList *pNext;
	StIOFIFOList *pPrev;
};

typedef struct _tagStIOFIFOCtrl
{
	StIOFIFOList *pUnusedList;
	int32_t s32Cnt;
	int32_t s32CurUsing;
	StIOFIFOList *pListRead;
	StIOFIFOList *pListWrite;
}StIOFIFOCtrl;



int32_t IOFIFOInit(StIOFIFOCtrl *pCtrl, StIOFIFOList *pList, int32_t s32Cnt, uint16_t u16ID);
StIOFIFOList *GetAUnusedFIFO(StIOFIFOCtrl *pCtrl);
int32_t InsertIntoTheRWFIFO(StIOFIFOCtrl *pCtrl, StIOFIFOList *pFIFO, bool boIsRead);
StIOFIFOList *GetAListFromRWFIFO(StIOFIFOCtrl *pCtrl, bool boIsRead);
int32_t ReleaseAUsedFIFO(StIOFIFOCtrl *pCtrl, StIOFIFOList *pFIFO);

typedef struct _tagStLevelOneCache
{
	char *pBuf;
	char *pBufBak;
	uint32_t u32TotalLength;
	uint32_t u32Write;
	uint32_t u32Read;
	uint32_t u32Using;
}StLevelOneCache;

int32_t LOCInit(StLevelOneCache *pLOC, void *pBuf, uint32_t u32Length); 
int32_t LOCWriteSomeData(StLevelOneCache *pLOC, void *pBuf, uint32_t u32Length);
void *LOCCheckDataCanRead(StLevelOneCache *pLOC, uint32_t *pLength);


typedef void (*PFUN_MessageInit)(void);
typedef StIOFIFO *(*PFUN_MessageFlush)(bool boSendALL);
typedef void (*PFUN_MessageRelease)(StIOFIFO *pFIFO);
typedef void (*PFUN_MessageReleaseNOReleaseData)(StIOFIFO *pFIFO);
typedef int32_t (*PFUN_GetMessageBufLength)(void);
typedef int32_t (*PFUN_MessageWrite)(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);

typedef struct _tagStIOTCB
{
	PFUN_MessageInit pFunMsgInit;
	PFUN_MessageFlush pFunMsgFlush;
	PFUN_MessageRelease pFunMsgRls;
	PFUN_MessageReleaseNOReleaseData pFunMsgRlsNRD;
	PFUN_GetMessageBufLength pFunGetMsgBufLength;
	PFUN_MessageWrite pFunMsgWrite;
}StIOTCB;



#endif
