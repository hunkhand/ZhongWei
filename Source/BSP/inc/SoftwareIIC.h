#ifndef __SOFTWAREIIC_H__
#define __SOFTWAREIIC_H__

//iic配置为SCL为PB6/SDA为PB7
#define SDA_IN()  {GPIOB->MODER &= ~(0x03<<14);}		//配置PB7为输入模式
#define SDA_OUT() {GPIOB->MODER |= 0x01<<14; GPIOB->OTYPER |= 0x00<<14; GPIOB->OSPEEDR |= 0x11<<14;}//配置PB7为输出模式

#define Set_IIC_SCL			GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define Reset_IIC_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_6)
#define Set_IIC_SDA			GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define Reset_IIC_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define READ_SDA			GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)
#define Write_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_7, x?Bit_SET:Bit_RESET)

//IIC所有操作函数
void Delay_Ms(u32 cnt);
void Delay_Us(u32 cnt);
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号
void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	

#endif
