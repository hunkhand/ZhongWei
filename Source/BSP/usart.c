#include "bsp.h"

void TIM7_IRQHandler(void);
void Timer7_Init(u8 u8BaudRate);

u8 UART1_RXBuff[MAX_USART1_DATA_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 UART1_RXBuffLen = 0;

u8 UART1_TXBUFF[MAX_USART1_DATA_LEN];


u8 u8SendIndex = 0;
u8 u8SendNum = 0;
extern OS_TCB  AppTaskMBTCB;


BitAction UartRecvNewData = Bit_RESET;
BitAction UartRecvFrameOK = Bit_RESET;

//初始化IO 串口1
//bound:波特率
void UART1_Init(UartCommTypeDef *pUartStructure)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    UartRecvNewData = Bit_RESET;
    UartRecvFrameOK = Bit_RESET;

    UART1_RXBuffLen = 0;

    u8SendIndex = 0;
    u8SendNum = 0;

    USART_ITConfig(USART1, USART_IT_TC, DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    USART_Cmd(USART1, DISABLE);

    NVIC_UARTConfiguration();

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_DeInit(USART1);    //复位串口1

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //USART 初始化设置
    switch (pUartStructure->BaudRate)
    {
    case 1:
      USART_InitStructure.USART_BaudRate = 2400;
    break;
    
    case 2:
      USART_InitStructure.USART_BaudRate = 4800;
    break;
    
    default:
      
    case 3:
      USART_InitStructure.USART_BaudRate = 9600;
    break;
		
    case 4:
      USART_InitStructure.USART_BaudRate = 19200;
    break;
		
    case 5:
      USART_InitStructure.USART_BaudRate = 38400;
    break;
		
    case 6:
      USART_InitStructure.USART_BaudRate = 57600;
    break;
		
    case 7:
      USART_InitStructure.USART_BaudRate = 115200;
    break;
    }
		
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 1个停止位
 	
    switch (pUartStructure->Parity)
    {
    case 1:
      USART_InitStructure.USART_Parity = USART_Parity_Odd;//奇校验位
      USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为9位数据格式
    break;
	    	
    case 2:
      USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验位
      USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为9位数据格式
    break;
		
    default:
		
    case 3:
      USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    break;
    }	

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制

    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure);   //初始化串口

    TX_OFF;

    BSP_IntVectSet(BSP_INT_ID_USART1, UART1_IRQService);

    USART_ClearFlag(USART1, USART_FLAG_RXNE);
    USART_ClearFlag(USART1, USART_FLAG_TXE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_ClearFlag(USART1, USART_FLAG_ORE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //开启中断

    USART_Cmd(USART1, ENABLE);                    //使能串口
    //if(UartComm.OutputMode != OUT_MODE_ASCII)
    //{
    //    Timer7_Init(pUartStructure->BaudRate);
    //}
}

void UART1_IRQService(void)                	//串口1中断服务程序
{
    //u8 UartReadTemp;
    OSIntEnter();   

    if (RESET != USART_GetITStatus(USART1, USART_IT_TC))
    {
        USART_ClearITPendingBit(USART1, USART_IT_TC);

        if (u8SendIndex >= u8SendNum)
        {
            u8SendNum = 0;
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
            TX_OFF;
        }
        else
        {
            USART_SendData(USART1, UART1_TXBUFF[u8SendIndex]);
            u8SendIndex++;
        }
    }

    //===============UART Recv===============//
    if (RESET != USART_GetITStatus(USART1, USART_IT_RXNE))
    {
	MODBUS_ASCII_HandlRevData(USART_ReceiveData(USART1));
    }

    if (RESET != USART_GetFlagStatus(USART1, USART_IT_ORE))
    {
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
    }

    OSIntExit();   
}


void TIM7_IRQHandler(void)
{
    OS_ERR  os_err;
    OSIntEnter();    
    
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

        if (Bit_SET == UartRecvNewData)
        {
            UartRecvNewData = Bit_RESET;
            UartRecvFrameOK = Bit_SET;

            TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM7, DISABLE);

            OSTaskSemPost((OS_TCB *)&AppTaskMBTCB, (OS_OPT) OS_OPT_POST_NONE,
                          (OS_ERR *)&os_err);
        }
        else
        {
            TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
            TIM_Cmd(TIM7, DISABLE);
        }
    }

    OSIntExit();
}

void Timer7_Init(u8 u8BaudRate)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;

    //使能定时器的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    //复位TIM2定时器，使之进入初始状态
    TIM_DeInit(TIM7);

    //先关闭定时器中断，接收到一个字符再打开
    TIM_Cmd(TIM7, DISABLE);

    NVIC_Timer7Configuration();

    //定时器工作模式，用于滤出高频干扰
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    //定时器计数模式  向上/向下/中心对称计数
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    //定时器周期

    if (u8BaudRate > 2)
    {
        TIM_TimeBaseInitStruct.TIM_Period = 1749;
    }
    else
    {
        TIM_TimeBaseInitStruct.TIM_Period = 6999 / (u8BaudRate + 1);
    }

    //定时器预分频因子
    //定时时间 T=(TIM_Prescaler+1)*(TIM_Period+1)/TIMxCLK
    TIM_TimeBaseInitStruct.TIM_Prescaler = 47;

    //初始化TIM2定时器
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);

    //初始化后挂起位立即有效，所以清除一下，否则一打开中断，马上进入中断
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

    BSP_IntVectSet(BSP_INT_ID_TIM7, TIM7_IRQHandler);

    //使能更新中断
    TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);

    TIM_Cmd(TIM7, DISABLE);

    TIM_SetCounter(TIM7, 0);

}

u32 UART1_SendData(u8* UART1_SendBuff, u32 Len)
{
    u32 i = 0;

    if ((0 == Len) || (((u8*)0) == UART1_SendBuff))
        return 0;

    if (u8SendNum != 0)
    {
        return 0;
    }

    if (Len > (sizeof(UART1_TXBUFF) / sizeof(UART1_TXBUFF[0])))
    {
        Len = (sizeof(UART1_TXBUFF) / sizeof(UART1_TXBUFF[0]));
    }

    for (i = 0; i < Len; i++)
    {
        UART1_TXBUFF[i] = UART1_SendBuff[i];
    }

    TX_ON;

    USART_SendData(USART1, UART1_TXBUFF[0]);

    u8SendIndex = 1;
    u8SendNum = Len;
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);

    return(Len);
}

s32 UART1_RecvData(u8* UART1_RecvBuff, u32 Len)
{
    u32 i = 0;

    if ((0 == Len) || (((u8*)NULL) == UART1_RecvBuff))
    {
        return 0;
    }

    if ((Bit_RESET == UartRecvFrameOK) || (0 == UART1_RXBuffLen))
    {
        return 0;
    }

    if (Len < UART1_RXBuffLen)
    {
        return -1;
    }

    Len = UART1_RXBuffLen;

    for (i = 0; i < Len; i++)
    {
        UART1_RecvBuff[i] = UART1_RXBuff[i];
    }

    UartRecvFrameOK = Bit_RESET;

    UART1_RXBuffLen = 0;

    return Len;
}

#include "string.h"
#include "stdarg.h"
#include "stdio.h"
void uprintf(const char *fmt,...)
{
#if 0
    va_list marker;
    char buff[64];
    memset(buff,0,sizeof(buff));
    va_start(marker, fmt);
    vsprintf(buff,fmt,marker);
    va_end(marker);
    UART1_SendData((uint8_t*)buff, strlen(buff));
    while(u8SendNum!=0);
# endif
}


