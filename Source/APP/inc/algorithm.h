#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "stm32f0xx.h"

void ArrayMerge(u8 A1[], u8 A2[], u8 A[], u8 Size1, u8 Size2);
void SortArrayExtreme(u32 Array[], const u32 ArraySize,
                      const u32 SortHeadSize, const u32 SortTailSize);
u32 GetDelExtremeAndAverage(u32 Array[], const u32 ArraySize,
                            const u32 SortHeadSize, const u32 SortTailSize);

u32 ArrToHex(u8 *pData);
float HexToFloat(u8 *pData);
void floatToHexArray(float fSource, u8 *pu8Temp, long num);
u32 HexToUlong(u8 *pData);
int get_slop(int *bufx, int *bufy, uint8_t len);
u32 Get_Min_Max(u32 *dBuf, u8 len, u8 mode);


#endif
