#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "app_port.h"
#include "IOCtrl.h"

#ifndef USE_CRITICAL

#define  USE_CRITICAL()
#define  ENTER_CRITICAL()
#define  EXIT_CRITICAL()
#endif // !USE_CRITICAL

int32_t IOFIFOInit(StIOFIFOCtrl *pCtrl, StIOFIFOList *pList, int32_t s32Cnt, uint16_t u16ID)
{
	int32_t i;
	USE_CRITICAL();
	if ((pCtrl == NULL) || (pList == NULL) || (s32Cnt < 0))
	{
		return -1;
	}

	ENTER_CRITICAL();
	
	memset(pList, 0, sizeof(StIOFIFOList) * s32Cnt);
	memset(pCtrl, 0, sizeof(StIOFIFOCtrl));

	for (i = 0; i < (s32Cnt - 1); i++)
	{
		pList[i].u16ID = u16ID;
		pList[i].pNext = pList + i + 1;
	}
	pList[i].pNext = NULL;

	pCtrl->s32Cnt = s32Cnt;
	pCtrl->pUnusedList = pList;

	EXIT_CRITICAL();
	return 0;
}

StIOFIFOList *GetAUnusedFIFO(StIOFIFOCtrl *pCtrl)
{
	USE_CRITICAL();
	StIOFIFOList *pFIFO = NULL;
	if (pCtrl == NULL)
	{
		return NULL;
	}
	ENTER_CRITICAL();

	if (pCtrl->pUnusedList != NULL)
	{
		pFIFO = pCtrl->pUnusedList;
		pCtrl->pUnusedList = pFIFO->pNext;
		pFIFO->pNext = pFIFO->pPrev = NULL;
	}
	EXIT_CRITICAL();
	return pFIFO;
}


int32_t InsertIntoTheRWFIFO(StIOFIFOCtrl *pCtrl, StIOFIFOList *pFIFO, bool boIsRead)
{
	if ((pCtrl == NULL) || (pFIFO == NULL))
	{
		return -1;
	}
	else
	{
		USE_CRITICAL();
		StIOFIFOList **p2Insert = boIsRead ?
			&(pCtrl->pListRead) : &(pCtrl->pListWrite);
		ENTER_CRITICAL();

		if (p2Insert[0] == NULL)
		{
			p2Insert[0] = pFIFO;
			pFIFO->pNext = pFIFO->pPrev = pFIFO;
		}
		else
		{
			pFIFO->pNext = p2Insert[0];
			p2Insert[0]->pPrev->pNext = pFIFO;
			pFIFO->pPrev = p2Insert[0]->pPrev;
			p2Insert[0]->pPrev = pFIFO;
		}
		EXIT_CRITICAL();
		return 0;
	}
}
StIOFIFOList *GetAListFromRWFIFO(StIOFIFOCtrl *pCtrl, bool boIsRead)
{
	USE_CRITICAL();
	StIOFIFOList **p2ListFrom;
	StIOFIFOList *pList = NULL;
	if (pCtrl == NULL)
	{
		return NULL;
	}
	p2ListFrom = boIsRead ? &(pCtrl->pListRead) : &(pCtrl->pListWrite);
	ENTER_CRITICAL();

	if (p2ListFrom[0] == NULL)
	{
		pList = NULL;
	}
	else
	{
		pList = p2ListFrom[0];
		/* just one element in the list */
		if (p2ListFrom[0]->pNext == p2ListFrom[0])
		{
			p2ListFrom[0] = NULL;
		}
		else
		{
			p2ListFrom[0]->pNext->pPrev = p2ListFrom[0]->pPrev;
			p2ListFrom[0]->pPrev->pNext = p2ListFrom[0]->pNext;
			p2ListFrom[0] = p2ListFrom[0]->pNext;
		}
		pList->pNext = pList->pPrev = NULL;
	}
	EXIT_CRITICAL();
	return pList;
}

int32_t ReleaseAUsedFIFO(StIOFIFOCtrl *pCtrl, StIOFIFOList *pFIFO)
{
	if ((pCtrl == NULL) ||(pFIFO == NULL))
	{
		return -1;
	}
	else
	{
		USE_CRITICAL();
		ENTER_CRITICAL();
		pFIFO->pNext = pCtrl->pUnusedList;
		pCtrl->pUnusedList = pFIFO;
		EXIT_CRITICAL();
		return 0;
	}
}

int32_t LOCInit(StLevelOneCache *pLOC, void *pBuf, uint32_t u32Length)
{
	USE_CRITICAL();
	if ((pLOC == NULL) || (pBuf == NULL))
	{
		return -1;
	}
	ENTER_CRITICAL();
	memset(pLOC, 0, sizeof(StLevelOneCache));
	pLOC->pBuf = pBuf;
	pLOC->u32TotalLength = u32Length / 2;
	pLOC->pBufBak = (char *)pBuf + pLOC->u32TotalLength;
	EXIT_CRITICAL();

	return 0;
}

int32_t LOCWriteSomeData(StLevelOneCache *pLOC, void *pBuf, uint32_t u32Length)
{
	USE_CRITICAL();
	uint32_t u32RemainLength;
	int32_t s32Err = 0;
	if ((pLOC == NULL) || (pBuf == NULL))
	{
		return -1;
	}
	ENTER_CRITICAL();
	u32RemainLength = pLOC->u32TotalLength - pLOC->u32Using;
	if (u32Length > u32RemainLength)
	{
		s32Err = - 1;
		goto end;
	}

	if (pLOC->u32Write + u32Length > pLOC->u32TotalLength)
	{
		u32RemainLength = pLOC->u32TotalLength - pLOC->u32Write;
		memcpy(pLOC->pBuf + pLOC->u32Write, pBuf, u32RemainLength);
		memcpy(pLOC->pBuf, (char *)pBuf + u32RemainLength, u32Length - u32RemainLength);
	}
	else
	{
		memcpy(pLOC->pBuf + pLOC->u32Write, pBuf, u32Length);
	}
	pLOC->u32Write += u32Length;
	pLOC->u32Write %= pLOC->u32TotalLength;
	pLOC->u32Using += u32Length;
end:
	EXIT_CRITICAL();
	return s32Err;
}

void *LOCCheckDataCanRead(StLevelOneCache *pLOC, uint32_t *pLength)
{
	USE_CRITICAL();

	if (pLOC == NULL)
	{
		return NULL;
	}

	if (pLOC->u32Using == 0)
	{
		return NULL;
	}

	ENTER_CRITICAL();

	if (pLOC->u32Read + pLOC->u32Using > pLOC->u32TotalLength)
	{
		int32_t s32Copy = pLOC->u32TotalLength - pLOC->u32Read;
		memcpy(pLOC->pBufBak, pLOC->pBuf + pLOC->u32Read, s32Copy);
		memcpy(pLOC->pBufBak + s32Copy, pLOC->pBuf, pLOC->u32Using - s32Copy);
	}
	else
	{
		memcpy(pLOC->pBufBak, pLOC->pBuf + pLOC->u32Read, pLOC->u32Using);
	}
	pLOC->u32Read += pLOC->u32Using;
	pLOC->u32Read %= pLOC->u32TotalLength;
	if (pLength != NULL)
	{
		*pLength = pLOC->u32Using;
	}
	pLOC->u32Using = 0;
	EXIT_CRITICAL();
	return pLOC->pBufBak;
}

