#include "BSP.h"

void Delay_Ms(u32 cnt)
{
    cnt = cnt * 7500;

    while (cnt--);
}

void Delay_Us(u32 cnt)
{
    cnt = cnt * 8;

    while (cnt--);
}

//???IIC
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    Set_IIC_SCL;
    Set_IIC_SDA;
}

//??IIC????
void IIC_Start(void)
{
    SDA_OUT();     //sda???
    Set_IIC_SDA;
    Set_IIC_SCL;
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
    Delay_Us(4);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    Reset_IIC_SDA;//START:when CLK is high,DATA change form high to low
    Delay_Us(4);
    Reset_IIC_SCL;//??I2C??,?????????
}

//??IIC????
void IIC_Stop(void)
{
    SDA_OUT();//sda???
    Reset_IIC_SCL;
    Reset_IIC_SDA;//STOP:when CLK is high DATA change form low to high
    Delay_Us(4);
    Set_IIC_SCL;
    Set_IIC_SDA;//??I2C??????
    Delay_Us(4);
}

//????????
//???:1,??????
//        0,??????
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      //SDA?????
    Set_IIC_SDA;
    Delay_Us(1);
    Set_IIC_SCL;
    Delay_Us(1);

    while (READ_SDA)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }

    Reset_IIC_SCL;//????0

    return 0;
}

//??ACK??
void IIC_Ack(void)
{
    Reset_IIC_SCL;
    SDA_OUT();
    Reset_IIC_SDA;
    Delay_Us(2);
    Set_IIC_SCL;
    Delay_Us(2);
    Reset_IIC_SCL;
}

//???ACK??
void IIC_NAck(void)
{
    Reset_IIC_SCL;
    SDA_OUT();
    Set_IIC_SDA;
    Delay_Us(2);
    Set_IIC_SCL;
    Delay_Us(2);
    Reset_IIC_SCL;
}

//IIC??????
//????????
// 1,???
// 0,???
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    Reset_IIC_SCL;//??????????

    for (t = 0;t < 8;t++)
    {
        Write_SDA((txd&0x80) >> 7);
        txd <<= 1;
        Delay_Us(2);   //?TEA5767??????????
        Set_IIC_SCL;
        Delay_Us(2);
        Reset_IIC_SCL;
        Delay_Us(2);
    }
}

//?1???,ack=1?,??ACK,ack=0,??nACK
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();//SDA?????

    for (i = 0;i < 8;i++)
    {
        Reset_IIC_SCL;
        Delay_Us(2);
        Set_IIC_SCL;
        receive <<= 1;

        if (READ_SDA)receive++;

        Delay_Us(1);
    }

    if (!ack)
        IIC_NAck();//??nACK
    else
        IIC_Ack(); //??ACK

    return receive;
}
