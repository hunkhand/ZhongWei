#ifndef __TYPE_H
#define __TYPE_H

#include "stm32f0xx.h"	
#include "Sys_cfg.h"
#include "os.h"

#define ECHO_ENABLE 	        0                                               //冻结
#define ECHO_DISABLE 	        1                                               //解冻

#define FLASH_WR_ENABLE         1                                               //Flash写使能
#define FLASH_WR_DISABLE        0                                               //Flash写禁止

#define PCAP_ADC_MAX_VALUE              65535

#define  COMPENSATE_ENABLE          0x01                                       //补偿使能
#define  COMPENSATE_DISABLE         0x00                                       //补偿禁止

typedef struct 
{
  u8 SlaveAddr;                                                                 //从机地址
  u8 BaudRate;                                                                  //波特率
  u8 WordLength;                                                                //数据位
  u8 StopBits;                                                                  //停止位
  u8 Parity;                                                                    //校验位
  u8 Echo;                                                                      //是否冻结
}UartCommTypeDef;

typedef struct
{
  u16 DACode[8];                                                                //DA码
  u16 Scale[8];                                                                 //百分度
}FloaterCalibTypeDef;

typedef struct
{
  u32 CapMin;                                                                   //电容零点
  u32 CapMax;                                                                   //电容满度
  u32 CapRange;                                                                 //电容范围       
  u16 Range;                                                                    //量程
  u16 SensorLen;                                                                 //传感器长度
  float DAMin;                                                                  //DA零点
  float DAMax;                                                                  //DA满度
  float Def_Range;                                                              //出厂量程
  float Def_CapMin;                                                             //出厂零点
  float Def_CapMax;                                                             //出厂满度
  
  float Temp_K;                                                                 //温度1系数K
  float Temp_B;                                                                 //温度1系数B
  float Temp2_K;                                                                //温度2系数K
  float Temp2_B;                                                                //温度2系数B
  float LiquitHeightforDAMax;                                                   //液位最大DA值
  float BottomArea;                                                             //底面积(单位：平方米) 
  float AddMapHeight;                                                           //加油阈值反映出来的高度（单位：mm）
  float SubMapHeight;                                                           //漏油阈值反映出来的高度（单位：mm）
  u8 AutoUpLoadTime;                                                             //自动上传周期
  u8 CompenEn;                                                                   //补偿使能
  //u8 OutputUnit;
  u8 bFlashWEn;                                                                 //写Flash使能
  u8 LiquidCmpTc;                                                               //液位比较时间间隔 2 3 4 5
  u8 FuelType;                                                                  //燃料类型
  u8 BoxShape;                                                                  //油箱形状
  u16 BoxPara[4];                                                               //油箱参数
  u16 AddLiqTime;                                                               //加油时间阈值
  u16 SubLiqTime;                                                               //漏油时间阈值
  u16 AddLiqCnt;                                                                //加油量阈值
  u16 SubLiqCnt;                                                                //漏油量阈值
  u16 RestLiqCnt;                                                                //二次加油阈值
  u16 AddOil;                                                                   //加油量
  u16 SubOil;                                                                   //漏油量
  u16 CalibTemp;                                                                //标定温度 保留 暂时无用
  u32 OilRange;                                                                 //油量量程
  u32 Oil_Add_Alarm;                                                            //加油极限报警
  u32 Oil_AddMinAlarm;                                                          //加油最小报警点
  FloaterCalibTypeDef Floater;                                                  //油浮标定类型结构
}ProductParaTypeDef;

typedef enum 
{
  Def_State = 0x00,                                                             //出厂标定状态
  CalibrateLow = 0x01,                                                          //标定零点
  CalibrateHeight = 0x02,                                                       //标定满度
  Calibrated = 0x03,                                                            //标定完成
}CalState_TypeDef;

typedef struct
{
  u8 OilInit;                                                                   //油量初始化标志
  u8 AccStatus;                                                                 //ACC状态
  u16 CarSpeed;                                                                 //汽车速度
  u16 FiltSpeed;                                                                //滤波速度
  u16 LowSpeedCount;                                                            //持续低速计数
  u16 DAForFloater;                                                             //油浮DA值
  u32 CarMileage;                                                               //里程
  u32 CapFromPCap;                                                              //电容
  u32 RawCap;                                                                   //原始电容
  u32 RawCapBak;                                                                //原始电容备份

  u32 OilQuantity;                                                              //邮箱油量
  u32 LiquidHeight;                                                             //液位高度值mm
  u16 LiquidAD;                                                                 //液位高度AD
  u16 LiquidPercent;                                                            //液位百分比
  float TempInSensor;                                                           //液体温度
  float TempInAir;                                                              //环境温度
  float HeightBak;
  
  CalState_TypeDef CalState;                                                    //标定状态结构体参数
  OS_TICK uartInitTime;                                                         //Uart初始化时间
  BitAction bInitializing;
  
  u32 Pvd_LiquidHeight;                                                      //断电瞬间的液位高度
  u32 Pvd_OilQuantity;                                                       //断电瞬间的油量
  u8  Pvd_Flag;       //PVD标志位
}RunVarTypeDef;

typedef struct
{
  u8 FilterLevel;                                                               //滤波等级
  u8 FilterBufMax;                                                              //滤波缓存最大值
  u8 FilterN;                                                                   //液位高度下限位（低于该值认为小波动，数据可更新）
  u8 FilterM;                                                                   //液位高度上限位（高于该值认为大波动，数据可更新）
  u16 FilterCycle;                                                              //滤波周期
  u32 HFil[10];                                                                 //高阶滤波数组
  u32 HFilBak[10];                                                              //高阶滤波数组备份
  u32 LFil[96];                                                                 //低阶滤波数组
  u32 LFilBak[96];                                                              //低阶滤波数组备份        
  u32 PCap_Filter;                                                              //滤波后的电容值
  float UserFlowK;                                                             //用户系数K
  float UserFlowB;                                                             //用户系数B
  
}UserParamTypeDef;



typedef struct
{
  u8 FilterLevel;                                                               //滤波等级
  u8 FilterBufMax;                                                              //滤波缓存最大值
  u8 FilterN;                                                                   //液位高度下限位（低于该值认为小波动，数据可更新）
  u8 FilterM;                                                                   //液位高度上限位（高于该值认为大波动，数据可更新）
  u16 FilterCycle;                                                              //滤波周期
  u32 HFil[10];                                                                 //高阶滤波数组
  u32 HFilBak[10];                                                              //高阶滤波数组备份
  u32 LFil[96];                                                                 //低阶滤波数组
  u32 LFilBak[96];                                                              //低阶滤波数组备份        
  u32 Pressure_Filter;                                                              //滤波后的电容值
  float UserFlowK;                                                             //用户系数K
  float UserFlowB;                                                             //用户系数B
  
}P_UserParamTypeDef;


/* PressureDataConvert做数据转换需要的参数结构 */
typedef struct {
    uint8_t CompenEn;                   //补偿使能
    uint16_t HeightRange;               //高度量程
    uint16_t PressureADMin;                  //压力AD值零点
    uint16_t PressureADLow;                  //压力AD值下刻度
    uint16_t PressureADHigh;                 //压力AD值上刻度
    uint16_t PressureADMax;                  //压力AD值满量程
    uint16_t PressureDAMin;                  //压力DA值零点
    uint16_t PressureDALow;                  //压力DA值下刻度
    uint16_t PressureDAHigh;                 //压力DA值上刻度
    uint16_t PressureDAMax;                  //压力DA值满量程
    uint32_t PressureMin;                    //压力零点
    uint32_t PressureMax;                    //压力满量程
    uint32_t PressureMinBak;                 //压力零点
    uint32_t PressureMaxBak;                 //压力满量程
    float Correct_K;                    //压力修正系数K
    float Correct_B;                    //压力修正系数B
}PressureDataConvert_Param;

/* PCap转换后输出数据的结构 */
typedef struct {
   
    u32 OilQuantity;                                                              //油箱油量          L
    u32 LiquidHeight;                                                             //液位高度值        0.1mm
    u16 LiquidHeightAD;                                                           //液位高度AD值     0-65535
    u16 LiquidPercent;                                                            //液位百分比       0-1000（0-1乘以1000）
    
    
    uint16_t PressureDA_ResultValue;        //PressureDA原始输出值
    uint16_t PressureDA_OutValue;           //PressureDA转换输出值
    uint32_t Pressure_ResultValue;          //Pressure原始值
}PressureDataConvert_Out_Param;




typedef struct
{
    BitAction FilterStart;                                                      //每秒滤波开始标志                                                           
    u8 Ms100_Cycle;                                                             //每1s获得电容次数计数                        
    u32 EverySecCap;                                                            //每秒的电容值                
    u32 FilArray[10];                                                           //每秒电容滤波数组
}EverySecFilTypeDef;


#endif

