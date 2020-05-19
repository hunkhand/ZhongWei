#include "bsp.h"

extern u8 UART1_RXBuff[MAX_USART1_DATA_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

extern BitAction UartRecvNewData ;
extern BitAction UartRecvFrameOK ;

u8 UsartErrCnt;
u8 cyAsciiBuff[MAX_USART1_DATA_LEN];

//static u8 g_cyRevState = ASCII_IDLE_STATE;
static u8 g_cyRevBufffLen = 0;


/************************************************************************************************************************************************************************
** 版权：   2015-2025, 深圳市信为科技发展有限公司
** 文件名:  Modbus_Ascii.h
** 作者:    陈锦
** 版本:    V1.0.0
** 日期:    2015-07-10
** 描述:    各种算法
** 功能:         
*************************************************************************************************************************************************************************
** 修改者:          No
** 版本:  		
** 修改内容:    No 
** 日期:            No
*************************************************************************************************************************************************************************/
//**************************************************************************************************
// 名称         : MODBUS_ASCII_HexToAscii()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : 十六进制数转ASCII码
// 输入参数     : 十六进制数(u8 cyHexData)
// 输出参数     : ASCII码(u8 *pCyAsciiBuf)
// 返回结果     : 无
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
void MODBUS_ASCII_HexToAscii(u8 cyHexData, u8 *pCyAsciiBuf)
{
	u8 cyTemp;
	
	cyTemp = cyHexData / 16;
	if (10 > cyTemp) //0-9
	{
		*(pCyAsciiBuf + 0) = cyTemp + '0';
	}
	else
	{
		*(pCyAsciiBuf + 0) = (cyTemp - 10) + 'A';
	}

	cyTemp = cyHexData % 16;
	if (10 > cyTemp) //0-9
	{
		*(pCyAsciiBuf + 1) = cyTemp + '0';
	}
	else
	{
		*(pCyAsciiBuf + 1) = (cyTemp - 10) + 'A';
	}
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_AsciiToHex()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : ASCII码转十六进制数
// 输入参数     : ASCII码(u8 *pCyAsciiBuf)
// 输出参数     : 无
// 返回结果     : 十六进制数(u8 cyHexData)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiToHex(u8 *pCyAsciiBuf)
{
	u8 cyHexData;
	
	cyHexData = 0;
	if ('A' > *(pCyAsciiBuf + 0) ) //0-9
	{
		cyHexData += *(pCyAsciiBuf + 0) - '0';
	}
	else if ('a' > *(pCyAsciiBuf + 0) ) //大写
	{
		cyHexData += *(pCyAsciiBuf + 0) - 'A' + 10;
	}
	else
	{
		cyHexData += *(pCyAsciiBuf + 0) - 'a' + 10;
	}

	cyHexData *= 16;
	
	if ('A' > *(pCyAsciiBuf + 1) ) //0-9
	{
		cyHexData += *(pCyAsciiBuf + 1) - '0';
	}
	else if ('a' > *(pCyAsciiBuf + 1) ) //大写
	{
		cyHexData += *(pCyAsciiBuf + 1) - 'A' + 10;
	}
	else
	{
		cyHexData += *(pCyAsciiBuf + 1) - 'a' + 10;
	}
	
	return (cyHexData);
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_GetLrc()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : 获取LRC值
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : LRC值(u8 cyLrcVal)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_GetLrc(u8 *pCyAsciiBuf, u8 cyLen)
{
	u8 i;
	u8 cyLrcVal;
	
	if (1 == (cyLen % 2) )
	{
		return 0;
	}
	
	cyLen /= 2;
	cyLrcVal = 0;
	for (i = 0; i < cyLen; i++)
	{
		cyLrcVal += MODBUS_ASCII_AsciiToHex(pCyAsciiBuf + i * 2);
	}
	//求补码
	cyLrcVal = ~cyLrcVal;
	cyLrcVal += 1;
	
	return (cyLrcVal);
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_AsciiPacketToRtuPacket()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : ASCII数据包转成RTU数据包
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf),  ASCII码串包长度(u8 cyAsciiLen)
// 输出参数     : RTU码串(u8 *pCyRtuBuf),
// 返回结果     : 0:错误；其他：RTU码串包长度(u8 cyRtuLen)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_AsciiPacketToRtuPacket(u8 *pCyAsciiBuf, u8 cyAsciiLen, u8 *pCyRtuBuf)
{
	u8 i;
	u8 cyRtuLen;
	
	if (1 == (cyAsciiLen % 2) )
	{
		return 0;
	}
	
	cyRtuLen = cyAsciiLen / 2;
	for (i = 0; i < cyRtuLen; i++)
	{
		*(pCyRtuBuf + i) = MODBUS_ASCII_AsciiToHex(pCyAsciiBuf + i * 2);
	}
	
	return (cyRtuLen);
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_RtuPacketToAsciiPacket()
// 创建日期     : 2015-07-24
// 作者         : 陈锦
// 功能         : RTU数据包转成ASCII数据包
// 输入参数     : RTU码串(u8 *pCyRtuBuf),  RTU码串包长度(u8 cyRtuLen)
// 输出参数     : ASCII码串(u8 *pCyAsciiBuf),
// 返回结果     : ASCII码串包长度(u8 cyAsciiLen)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************

u8 MODBUS_ASCII_RtuPacketToAsciiPacket(u8 *pCyRtuBuf, u8 cyRtuLen, u8 *pCyAsciiBuf)
{
	u8 i;
	u8 cyAsciiLen;
	
	cyAsciiLen = cyRtuLen * 2;
	for (i = 0; i < cyRtuLen; i++)
	{
		MODBUS_ASCII_HexToAscii( *(pCyRtuBuf + i), pCyAsciiBuf + i * 2);
	}
	
	return (cyAsciiLen);
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_HandlRevData()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : ASCII处理接收数据
// 输入参数     : 接收数据(u8 cyRevData)
// 输出参数     : 无
// 返回结果     : 无
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************
void MODBUS_ASCII_HandlRevData(u8 cyRevData)
{
    OS_ERR  os_err;
    
    if(g_cyRevBufffLen > MAX_USART1_DATA_LEN)
    {
      //UsartErrCnt = 30;
      g_cyRevBufffLen = 0;
      //USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
      //USART_Cmd(USART1, DISABLE);
      return;
    }
    
    if(0x3A == cyRevData)
    {
      g_cyRevBufffLen = 0;
      UART1_RXBuff[g_cyRevBufffLen++] = cyRevData;
    }
    else if((0x0A == cyRevData)&&(3 < g_cyRevBufffLen))
    {
      if(0x0D == UART1_RXBuff[g_cyRevBufffLen - 1])
      {
        UART1_RXBuff[g_cyRevBufffLen++] = cyRevData;
        
        UartRecvFrameOK = Bit_SET;
        OSTaskSemPost((OS_TCB *)&AppTaskMBTCB, (OS_OPT) OS_OPT_POST_NONE,
                          (OS_ERR *)&os_err);
      }
    }
    else
    {
      UART1_RXBuff[g_cyRevBufffLen++] = cyRevData;
    }
    
    /*switch(g_cyRevState)
    {
        case ASCII_HEAD_STATE:  
          if (ASCII_HEAD_DATA == cyRevData)
          {
                  g_cyRevBufffLen = 0;
          }
          else if (0x0D == cyRevData)
          {
                  g_cyRevState = ASCII_END_STATE;
          }
          UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
          g_cyRevBufffLen++;
          if (MAX_USART1_DATA_LEN <= g_cyRevBufffLen)
          {
                  g_cyRevState = ASCII_IDLE_STATE;
          }
          break;
          
          case ASCII_END_STATE:   
            if (ASCII_HEAD_DATA == cyRevData)
            {
                  g_cyRevBufffLen = 0;
                  g_cyRevState = ASCII_HEAD_STATE;
                  UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
                  g_cyRevBufffLen++;
            }
            else if (0x0A == cyRevData)
            {
                    g_cyRevState = ASCII_IDLE_STATE;
                    
                    UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
                    g_cyRevBufffLen++;
    
                    //发送成功接收一包数据的标志
                    UartRecvFrameOK = Bit_SET;
					
		   OSTaskSemPost((OS_TCB *)&AppTaskMBTCB, (OS_OPT) OS_OPT_POST_NONE,
                          (OS_ERR *)&os_err);
            }
            else
            {
                    g_cyRevState = ASCII_IDLE_STATE;
            }
            break; 
            
            case ASCII_IDLE_STATE:  
            if (ASCII_HEAD_DATA == cyRevData)
            {
                    g_cyRevBufffLen = 0;
                    g_cyRevState = ASCII_HEAD_STATE;
                    UART1_RXBuff[g_cyRevBufffLen] = cyRevData;
                    g_cyRevBufffLen++;
            }
            break;
           default:               
             g_cyRevState = ASCII_IDLE_STATE; 
           break;
    }*/
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_CheckAscii()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : 检验是否都是Ascii码
// 输入参数     : ASCII码串(u8 *pCyAsciiBuf), 数据长度(u8 cyLen)
// 输出参数     : 无
// 返回结果     : 检测(0 不全是， 1 全是)
// 注意和说明   : 
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_CheckAscii(u8 *pCyAsciiBuf, u8 cyLen)
{
	u8 i;
	
	for (i = 0; i < cyLen; i++)
	{
		if ('0' > *(pCyAsciiBuf + i) )
		{
			break;
		}
		
		if ( ('9' < *(pCyAsciiBuf + i) ) && ( *(pCyAsciiBuf + i) < 'A' ) )
		{
			break;
		}
		
		if ( ('F' < *(pCyAsciiBuf + i) ) && ( *(pCyAsciiBuf + i) < 'a' ) )
		{
			break;
		}
		
		if ('f' < *(pCyAsciiBuf + i) )
		{
			break;
		}
	}
	
	
	if (i == cyLen)
	{
		return (1);
	}
	
	return (0);
}

//**************************************************************************************************
// 名称         : MODBUS_ASCII_RecvData()
// 创建日期     : 2015-07-27
// 作者         : 陈锦
// 功能         : 接收一串数据
// 输入参数     : 数据串(cyRecvBuff)
// 输出参数     : 数据长度(u8 *cyLen)
// 返回结果     : 执行结果(0 没有接收数据， 1 接收数据出现不是ASCII码， 2 效验码错误， 3 成功)
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************

u8 MODBUS_ASCII_RecvData(u8* cyRecvBuff, u8 *pCyLen)
{
    u8 cyLrc;
	
    if (((u8*)NULL) == cyRecvBuff)
    {
        return 0;
    }

    if ((Bit_RESET == UartRecvFrameOK) || (0 == g_cyRevBufffLen))
    {
        return 0;
    }
    
    UartRecvFrameOK = Bit_RESET;
    
    if (0 == MODBUS_ASCII_CheckAscii(&UART1_RXBuff[1], g_cyRevBufffLen - 3) )
    {
    	//发送数据错误
    	return 1;
    }

    cyLrc = MODBUS_ASCII_GetLrc(&UART1_RXBuff[1], g_cyRevBufffLen - 5);
    if (cyLrc != MODBUS_ASCII_AsciiToHex(&UART1_RXBuff[g_cyRevBufffLen - 4]) )
    {
    	//发送数据效验错误
    	return 2;
    }

    *pCyLen = MODBUS_ASCII_AsciiPacketToRtuPacket(&UART1_RXBuff[1], g_cyRevBufffLen - 5, cyRecvBuff);
	
    return (3);
}


//**************************************************************************************************
// 名称         : MODBUS_ASCII_SendData()
// 创建日期     : 2015-07-13
// 作者         : 陈锦
// 功能         : 发送一串数据
// 输入参数     : 数据串(u8 *cySendBuff), 数据长度(cyLen) (数据长度 小于 123)
// 输出参数     : 无
// 返回结果     : 执行结果(0 失败， 数据长度 成功)
// 注意和说明   :
// 修改内容     :
//**************************************************************************************************
u8 MODBUS_ASCII_SendData(u8 *cySendBuff, u8 cyLen)
{
    u8 cyLrc;
    u8 cyAsciiLen;
	
    if ( (0 == cyLen) || ( ((u8*)NULL) == cySendBuff) )
    {
        return 0;
    }
    
    if ( (cyLen * 2 + 5) > MAX_USART1_DATA_LEN)
    {
    	return 0;
    }
    
    cyAsciiBuff[0] = ASCII_HEAD_DATA;
    cyAsciiLen = 1;
    
    cyAsciiLen += MODBUS_ASCII_RtuPacketToAsciiPacket(cySendBuff, cyLen, &cyAsciiBuff[1]);
    cyLrc = MODBUS_ASCII_GetLrc(&cyAsciiBuff[1], cyAsciiLen - 1);
    MODBUS_ASCII_HexToAscii(cyLrc, &cyAsciiBuff[cyAsciiLen]);
    cyAsciiLen += 2;
    cyAsciiBuff[cyAsciiLen] = 0x0D;
    cyAsciiLen++;
    cyAsciiBuff[cyAsciiLen] = 0x0A;
    cyAsciiLen++;
    
    return ( UART1_SendData(cyAsciiBuff, cyAsciiLen) );
}
