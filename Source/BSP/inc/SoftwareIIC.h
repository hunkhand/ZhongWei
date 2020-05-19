#ifndef __SOFTWAREIIC_H__
#define __SOFTWAREIIC_H__

//iic����ΪSCLΪPB6/SDAΪPB7
#define SDA_IN()  {GPIOB->MODER &= ~(0x03<<14);}		//����PB7Ϊ����ģʽ
#define SDA_OUT() {GPIOB->MODER |= 0x01<<14; GPIOB->OTYPER |= 0x00<<14; GPIOB->OSPEEDR |= 0x11<<14;}//����PB7Ϊ���ģʽ

#define Set_IIC_SCL			GPIO_SetBits(GPIOB, GPIO_Pin_6)
#define Reset_IIC_SCL		GPIO_ResetBits(GPIOB, GPIO_Pin_6)
#define Set_IIC_SDA			GPIO_SetBits(GPIOB, GPIO_Pin_7)
#define Reset_IIC_SDA		GPIO_ResetBits(GPIOB, GPIO_Pin_7)
#define READ_SDA			GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7)
#define Write_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_7, x?Bit_SET:Bit_RESET)

//IIC���в�������
void Delay_Ms(u32 cnt);
void Delay_Us(u32 cnt);
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�
void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	

#endif
