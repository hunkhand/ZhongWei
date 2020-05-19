#ifndef _MODBUS_ASCII_H
#define _MODBUS_ASCII_H

#define ASCII_HEAD_STATE  0     //头
#define ASCII_END_STATE   1     //结束
#define ASCII_IDLE_STATE  2     //空闲

#define ASCII_HEAD_DATA   ':'           //包头第一个字节

void MODBUS_ASCII_HexToAscii(u8 cyHexData, u8 *pCyAsciiBuf);
u8 MODBUS_ASCII_AsciiToHex(u8 *pCyAsciiBuf);
u8 MODBUS_ASCII_GetLrc(u8 *pCyAsciiBuf, u8 cyLen);
u8 MODBUS_ASCII_AsciiPacketToRtuPacket(u8 *pCyAsciiBuf, u8 cyAsciiLen, u8 *pCyRtuBuf);
u8 MODBUS_ASCII_RtuPacketToAsciiPacket(u8 *pCyRtuBuf, u8 cyRtuLen, u8 *pCyAsciiBuf);
void MODBUS_ASCII_HandlRevData(u8 cyRevData);
u8 MODBUS_ASCII_CheckAscii(u8 *pCyAsciiBuf, u8 cyLen);
u8 MODBUS_ASCII_RecvData(u8* cyRecvBuff, u8 *pCyLen);
u8 MODBUS_ASCII_SendData(u8 *cySendBuff, u8 cyLen);

#endif