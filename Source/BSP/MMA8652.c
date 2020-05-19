#include "MMA8652.h"

void Delay2p5us(void)
{
	volatile unsigned long i = 30;
	while(--i)
	{
		;
	}
}

signed int  xvalue=0;
signed int  yvalue=0;
signed int  zvalue=0;

float xg,yg,zg;//加速度分量
float angle1,angle2,angle3;//角度

uchar BUF[8]; //接收数据缓存区  

#define CTRL_REG1  0x2A
#define CTRL_REG2  0x2B
#define CTRL_REG3  0x2C
#define CTRL_REG4  0x2D
#define CTRL_REG5  0x2E
#define INT_SOURCE_REG 0x0c

void MMA8652_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);

	//SCL==>PB13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//SDA==>PB14
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void MMA8652_Setting(void)
{
	// Go to the Standby Mode
	Single_Write_MMA8652(CTRL_REG1, 0x00 | (0x7 << 3));
	// Configure the INT pins for Open Drain and Active Low
    Single_Write_MMA8652(CTRL_REG3, 0x01);
	// Enable the Data Ready Interrupt and route it to INT2
	Single_Write_MMA8652(CTRL_REG4, 0x01);
	Single_Write_MMA8652(CTRL_REG5, 0X00);
	
	// Go back to Active Mode
	Single_Write_MMA8652(CTRL_REG1, 0x01 | (0x7 << 3));
}

/**************************************
起始信号
**************************************/
void MMA8652_Start(void)
{
    SDA_1;        //拉高数据线
    SCL_1;        //拉高时钟线
    Delay2p5us(); //延时
    SDA_0;        //产生下降沿
    Delay2p5us(); //延时
    SCL_0;        //拉低时钟线
    Delay2p5us(); //延时
}

/**************************************
停止信号
**************************************/
void MMA8652_Stop(void)
{
    SDA_0;      	//拉低数据线
    SCL_1;      	//拉高时钟线
    Delay2p5us();	//延时
    SDA_1;      	//产生上升沿
    Delay2p5us();   //延时
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void MMA8652_SendACK(char ack)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_14, (BitAction)(ack));//写应答信号
    SCL_1;        	//拉高时钟线
    Delay2p5us();   //延时
    SCL_0;        	//拉低时钟线
    Delay2p5us();   //延时
}

/**************************************
接收应答信号
**************************************/
char MMA8652_RecvACK(void)
{
  char ack=0;
  
  SCL_1; 
  Delay2p5us(); 
  ack = (char)GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14);
  SCL_0; 
  Delay2p5us();
  return ack;
}

/**************************************
向IIC总线发送一个字节数据
**************************************/
void MMA8652_SendByte(char dat)
{
	char i,temp;

	for(i=0;i<8;i++)
	{
		temp = dat&0x80;
		if(temp == 0)
		{
			//GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction)0);
			GPIO_ResetBits(GPIOB,GPIO_Pin_14);
		}
		else
		{
			//GPIO_WriteBit(GPIOB,GPIO_Pin_14,(BitAction)1);
			GPIO_SetBits(GPIOB,GPIO_Pin_14);
		}
		SCL_1;
		Delay2p5us();
		SCL_0;
		Delay2p5us(); 
		dat <<= 1;
	}
	MMA8652_RecvACK();
}

/**************************************
从IIC总线接收一个字节数据
**************************************/
char MMA8652_RecvByte(void)
{
	char i;
	char dat = 0;

	SDA_1; 
	for(i=0;i<8;i++)
	{
		dat <<= 1;
		SCL_1;   
		Delay2p5us(); 
		dat |= GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14); 
		SCL_0; 
		Delay2p5us();  
	}
	return dat;
}

//******单字节写入*******************************************
void Single_Write_MMA8652(uchar REG_Address,uchar REG_data)
{
  MMA8652_Start(); //起始信号
  MMA8652_SendByte(MMA845x_I2C_ADDRESS);//发送设备地址+写信号
  MMA8652_SendByte(REG_Address);    //内部寄存器地址
  MMA8652_SendByte(REG_data);       //内部寄存器数据
  MMA8652_Stop();                   //发送停止信号
}

//********单字节读取*****************************************
uchar Single_Read_MMA8652(uchar REG_Address)
{
  uchar REG_data;
  
  MMA8652_Start(); //起始信号
  MMA8652_SendByte(MMA845x_I2C_ADDRESS);//发送设备地址+写信号
  MMA8652_SendByte(REG_Address); //发送存储单元地址，从0开始	
  MMA8652_Start(); //起始信号
  MMA8652_SendByte(MMA845x_I2C_ADDRESS+1);//发送设备地址+读信号
  REG_data=MMA8652_RecvByte();     //读出寄存器数据
  MMA8652_SendACK(1);
  MMA8652_Stop();  //停止信号
  return REG_data; 
}

//*********************************************************
//
//连续读出MMA8652内部加速度数据，地址范围0x01~0x06
//
//*********************************************************
void Multiple_read_MMA8652(ACCEL_TypeDef *pAccel)
{
	char i;
	unsigned char BUF[6]; //接收数据缓存区  

	MMA8652_Start(); //起始信号
	MMA8652_SendByte(MMA845x_I2C_ADDRESS);//发送设备地址+写信号
	MMA8652_SendByte(0x01);//发送存储单元地址，从0x01开始	
	MMA8652_Start();//起始信号
	MMA8652_SendByte(MMA845x_I2C_ADDRESS + 1);//发送设备地址+读信号
	for(i=0; i<6; i++)//连续读取6个地址数据，存储中BUF
	{
		BUF[i] = MMA8652_RecvByte(); //BUF[0]存储0x32地址中的数据
		if(i == 5)
		{
		  MMA8652_SendACK(1); //最后一个数据需要回NOACK
		}
		else
		{
		  MMA8652_SendACK(0); //回应ACK
		}
	}
	MMA8652_Stop();  //停止信号
	
	pAccel->X_Value = (BUF[0]<<8)|BUF[1];
	pAccel->Y_Value = (BUF[2]<<8)|BUF[3];
	pAccel->Z_Value = (BUF[4]<<8)|BUF[5];
	
	if(BUF[0]&0x80) pAccel->X_Value = ~pAccel->X_Value + 1;
	if(BUF[2]&0x80) pAccel->Y_Value = ~pAccel->Y_Value + 1;
	if(BUF[4]&0x80) pAccel->Z_Value = ~pAccel->Z_Value + 1;
	
	pAccel->X_Value >>= 4;
	pAccel->Y_Value >>= 4;
	pAccel->Z_Value >>= 4;
}

void getAngle(ACCEL_TypeDef *pAccel, ANGLE_TypeDef *pAngle)
{
	float total;
	float ftemp;
	BitAction bXNegetive = Bit_RESET;
	BitAction bZNegetive = Bit_RESET;

	if(pAccel->X_Value<0)
	{
		pAccel->X_Value = 4096 + pAccel->X_Value;
		pAccel->X_Value = -pAccel->X_Value;
		bXNegetive = Bit_SET;
	}
	if(pAccel->Y_Value<0)
	{
		pAccel->Y_Value = 4096 + pAccel->Y_Value;
		pAccel->Y_Value = -pAccel->Y_Value;   
	}
	if(pAccel->Z_Value<0)
	{
		pAccel->Z_Value = 4096 + pAccel->Z_Value;
		pAccel->Z_Value = -pAccel->Z_Value;    
		bZNegetive = Bit_SET;
	}
	
	total = pAccel->X_Value * pAccel->X_Value + 
		    pAccel->Y_Value * pAccel->Y_Value + 
		    pAccel->Z_Value * pAccel->Z_Value;
	
	total = sqrt(total);
	
	ftemp = pAccel->X_Value / total;
	pAngle->XtoG = acos(ftemp) * 180 / 3.1415926;
	
	ftemp = pAccel->Y_Value / total;
	pAngle->YtoG = acos(ftemp) * 180 / 3.1415926;
	
	ftemp = pAccel->Z_Value / total;
	pAngle->ZtoG = 180.00 - acos(ftemp) * 180 / 3.1415926;

	if(Bit_SET == bXNegetive)
	{
		pAngle->ZtoG = 0 - pAngle->ZtoG;
	}
	if(Bit_SET == bZNegetive)
	{
		pAngle->XtoG = 0.0 - pAngle->XtoG;
		pAngle->YtoG = 0.0 - pAngle->XtoG;
	}
}
