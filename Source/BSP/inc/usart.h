#ifndef _USART_H
#define _USART_H
#include "stdio.h"

//extern u8  *USART_RX_BUF; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
//extern u16 UART1_RXBuffLen;
#include "type.h"
#define MAX_USART1_DATA_LEN   210   //发送接收最长的数据 (该值小于256）

#define TX_ON    GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define TX_OFF   GPIO_ResetBits(GPIOA, GPIO_Pin_8)

void UART1_Init(UartCommTypeDef *pUartStructure);
void UART1_IRQService( void ) ;
u32 UART1_SendData( u8* UART1_SendBuff, u32 Len );
s32 UART1_RecvData( u8* UART1_RecvBuff, u32 Len );

void uprintf(const char *fmt,...);

#endif


