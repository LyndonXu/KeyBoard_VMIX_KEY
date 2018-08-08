#ifndef COMMON_H_
#define COMMON_H_
#include <stdint.h>

void LittleAndBigEndianTransfer(char *pDest, const char *pSrc, uint32_t u32Size);
uint16_t CRC16(const uint8_t *pFrame, uint16_t u16Len);
uint32_t CRC32Buf(uint8_t *pBuf, uint32_t u32Length);
int32_t btea(int32_t *v, int32_t n, const int32_t *k);

#endif
