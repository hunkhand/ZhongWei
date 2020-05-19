#ifndef _MMA8652_H_
#define _MMA8652_H_

#include "stm32f0xx.h"
#include "math.h"

typedef struct{
    float XtoG;
	float YtoG;
	float ZtoG;
}ANGLE_TypeDef;

typedef struct{
    signed int X_Value;
	signed int Y_Value;
	signed int Z_Value;
}ACCEL_TypeDef;

//=============SCL===============================
#define  SCL_1 GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define  SCL_0 GPIO_ResetBits(GPIOB,GPIO_Pin_13)
//=============SDA===============================
#define  SDA_1 GPIO_SetBits(GPIOB,GPIO_Pin_14)
#define  SDA_0 GPIO_ResetBits(GPIOB,GPIO_Pin_14)

#define	MMA845x_I2C_ADDRESS       (0x1D<<1) 	 //定义器件在IIC总线中的从地址,根据SA0=1为0x1D
//#define MMA845x_I2C_ADDRESS     0x1D   // SA0 = 1
//#define MMA845x_I2C_ADDRESS    0x1C    // SA0 = 0
typedef unsigned char  uchar;
typedef unsigned char  BYTE;
typedef unsigned short WORD;

extern signed int  xvalue;
extern signed int  yvalue;
extern signed int  zvalue;
extern uchar BUF[8];//接收数据缓存区  

extern float xg,yg,zg;//加速度分量
extern float total;
extern float angle1,angle2,angle3;//角度

void MMA8652_GPIO_Config(void);
void MMA8652_Setting(void);
void MMA8652_Start(void);
void MMA8652_Stop(void);
void MMA8652_SendACK(char ack);
char MMA8652_RecvACK(void);
void MMA8652_SendByte(char dat);
char MMA8652_RecvByte(void);
void Single_Write_MMA8652(uchar REG_Address,uchar REG_data);
uchar Single_Read_MMA8652(uchar REG_Address);
void Multiple_read_MMA8652(ACCEL_TypeDef *pAccel);
void getAngle(ACCEL_TypeDef *pAccel, ANGLE_TypeDef *pAngle);


#endif