#ifndef __TYPE_H
#define __TYPE_H

#include "stm32f0xx.h"	
#include "Sys_cfg.h"
#include "os.h"

#define ECHO_ENABLE 	        0                                               //����
#define ECHO_DISABLE 	        1                                               //�ⶳ

#define FLASH_WR_ENABLE         1                                               //Flashдʹ��
#define FLASH_WR_DISABLE        0                                               //Flashд��ֹ

#define PCAP_ADC_MAX_VALUE              65535

#define  COMPENSATE_ENABLE          0x01                                       //����ʹ��
#define  COMPENSATE_DISABLE         0x00                                       //������ֹ

typedef struct 
{
  u8 SlaveAddr;                                                                 //�ӻ���ַ
  u8 BaudRate;                                                                  //������
  u8 WordLength;                                                                //����λ
  u8 StopBits;                                                                  //ֹͣλ
  u8 Parity;                                                                    //У��λ
  u8 Echo;                                                                      //�Ƿ񶳽�
}UartCommTypeDef;

typedef struct
{
  u16 DACode[8];                                                                //DA��
  u16 Scale[8];                                                                 //�ٷֶ�
}FloaterCalibTypeDef;

typedef struct
{
  u32 CapMin;                                                                   //�������
  u32 CapMax;                                                                   //��������
  u32 CapRange;                                                                 //���ݷ�Χ       
  u16 Range;                                                                    //����
  u16 SensorLen;                                                                 //����������
  float DAMin;                                                                  //DA���
  float DAMax;                                                                  //DA����
  float Def_Range;                                                              //��������
  float Def_CapMin;                                                             //�������
  float Def_CapMax;                                                             //��������
  
  float Temp_K;                                                                 //�¶�1ϵ��K
  float Temp_B;                                                                 //�¶�1ϵ��B
  float Temp2_K;                                                                //�¶�2ϵ��K
  float Temp2_B;                                                                //�¶�2ϵ��B
  float LiquitHeightforDAMax;                                                   //Һλ���DAֵ
  float BottomArea;                                                             //�����(��λ��ƽ����) 
  float AddMapHeight;                                                           //������ֵ��ӳ�����ĸ߶ȣ���λ��mm��
  float SubMapHeight;                                                           //©����ֵ��ӳ�����ĸ߶ȣ���λ��mm��
  u8 AutoUpLoadTime;                                                             //�Զ��ϴ�����
  u8 CompenEn;                                                                   //����ʹ��
  //u8 OutputUnit;
  u8 bFlashWEn;                                                                 //дFlashʹ��
  u8 LiquidCmpTc;                                                               //Һλ�Ƚ�ʱ���� 2 3 4 5
  u8 FuelType;                                                                  //ȼ������
  u8 BoxShape;                                                                  //������״
  u16 BoxPara[4];                                                               //�������
  u16 AddLiqTime;                                                               //����ʱ����ֵ
  u16 SubLiqTime;                                                               //©��ʱ����ֵ
  u16 AddLiqCnt;                                                                //��������ֵ
  u16 SubLiqCnt;                                                                //©������ֵ
  u16 RestLiqCnt;                                                                //���μ�����ֵ
  u16 AddOil;                                                                   //������
  u16 SubOil;                                                                   //©����
  u16 CalibTemp;                                                                //�궨�¶� ���� ��ʱ����
  u32 OilRange;                                                                 //��������
  u32 Oil_Add_Alarm;                                                            //���ͼ��ޱ���
  u32 Oil_AddMinAlarm;                                                          //������С������
  FloaterCalibTypeDef Floater;                                                  //�͸��궨���ͽṹ
}ProductParaTypeDef;

typedef enum 
{
  Def_State = 0x00,                                                             //�����궨״̬
  CalibrateLow = 0x01,                                                          //�궨���
  CalibrateHeight = 0x02,                                                       //�궨����
  Calibrated = 0x03,                                                            //�궨���
}CalState_TypeDef;

typedef struct
{
  u8 OilInit;                                                                   //������ʼ����־
  u8 AccStatus;                                                                 //ACC״̬
  u16 CarSpeed;                                                                 //�����ٶ�
  u16 FiltSpeed;                                                                //�˲��ٶ�
  u16 LowSpeedCount;                                                            //�������ټ���
  u16 DAForFloater;                                                             //�͸�DAֵ
  u32 CarMileage;                                                               //���
  u32 CapFromPCap;                                                              //����
  u32 RawCap;                                                                   //ԭʼ����
  u32 RawCapBak;                                                                //ԭʼ���ݱ���

  u32 OilQuantity;                                                              //��������
  u32 LiquidHeight;                                                             //Һλ�߶�ֵmm
  u16 LiquidAD;                                                                 //Һλ�߶�AD
  u16 LiquidPercent;                                                            //Һλ�ٷֱ�
  float TempInSensor;                                                           //Һ���¶�
  float TempInAir;                                                              //�����¶�
  float HeightBak;
  
  CalState_TypeDef CalState;                                                    //�궨״̬�ṹ�����
  OS_TICK uartInitTime;                                                         //Uart��ʼ��ʱ��
  BitAction bInitializing;
  
  u32 Pvd_LiquidHeight;                                                      //�ϵ�˲���Һλ�߶�
  u32 Pvd_OilQuantity;                                                       //�ϵ�˲�������
  u8  Pvd_Flag;       //PVD��־λ
}RunVarTypeDef;

typedef struct
{
  u8 FilterLevel;                                                               //�˲��ȼ�
  u8 FilterBufMax;                                                              //�˲��������ֵ
  u8 FilterN;                                                                   //Һλ�߶�����λ�����ڸ�ֵ��ΪС���������ݿɸ��£�
  u8 FilterM;                                                                   //Һλ�߶�����λ�����ڸ�ֵ��Ϊ�󲨶������ݿɸ��£�
  u16 FilterCycle;                                                              //�˲�����
  u32 HFil[10];                                                                 //�߽��˲�����
  u32 HFilBak[10];                                                              //�߽��˲����鱸��
  u32 LFil[96];                                                                 //�ͽ��˲�����
  u32 LFilBak[96];                                                              //�ͽ��˲����鱸��        
  u32 PCap_Filter;                                                              //�˲���ĵ���ֵ
  float UserFlowK;                                                             //�û�ϵ��K
  float UserFlowB;                                                             //�û�ϵ��B
  
}UserParamTypeDef;



typedef struct
{
  u8 FilterLevel;                                                               //�˲��ȼ�
  u8 FilterBufMax;                                                              //�˲��������ֵ
  u8 FilterN;                                                                   //Һλ�߶�����λ�����ڸ�ֵ��ΪС���������ݿɸ��£�
  u8 FilterM;                                                                   //Һλ�߶�����λ�����ڸ�ֵ��Ϊ�󲨶������ݿɸ��£�
  u16 FilterCycle;                                                              //�˲�����
  u32 HFil[10];                                                                 //�߽��˲�����
  u32 HFilBak[10];                                                              //�߽��˲����鱸��
  u32 LFil[96];                                                                 //�ͽ��˲�����
  u32 LFilBak[96];                                                              //�ͽ��˲����鱸��        
  u32 Pressure_Filter;                                                              //�˲���ĵ���ֵ
  float UserFlowK;                                                             //�û�ϵ��K
  float UserFlowB;                                                             //�û�ϵ��B
  
}P_UserParamTypeDef;


/* PressureDataConvert������ת����Ҫ�Ĳ����ṹ */
typedef struct {
    uint8_t CompenEn;                   //����ʹ��
    uint16_t HeightRange;               //�߶�����
    uint16_t PressureADMin;                  //ѹ��ADֵ���
    uint16_t PressureADLow;                  //ѹ��ADֵ�¿̶�
    uint16_t PressureADHigh;                 //ѹ��ADֵ�Ͽ̶�
    uint16_t PressureADMax;                  //ѹ��ADֵ������
    uint16_t PressureDAMin;                  //ѹ��DAֵ���
    uint16_t PressureDALow;                  //ѹ��DAֵ�¿̶�
    uint16_t PressureDAHigh;                 //ѹ��DAֵ�Ͽ̶�
    uint16_t PressureDAMax;                  //ѹ��DAֵ������
    uint32_t PressureMin;                    //ѹ�����
    uint32_t PressureMax;                    //ѹ��������
    uint32_t PressureMinBak;                 //ѹ�����
    uint32_t PressureMaxBak;                 //ѹ��������
    float Correct_K;                    //ѹ������ϵ��K
    float Correct_B;                    //ѹ������ϵ��B
}PressureDataConvert_Param;

/* PCapת����������ݵĽṹ */
typedef struct {
   
    u32 OilQuantity;                                                              //��������          L
    u32 LiquidHeight;                                                             //Һλ�߶�ֵ        0.1mm
    u16 LiquidHeightAD;                                                           //Һλ�߶�ADֵ     0-65535
    u16 LiquidPercent;                                                            //Һλ�ٷֱ�       0-1000��0-1����1000��
    
    
    uint16_t PressureDA_ResultValue;        //PressureDAԭʼ���ֵ
    uint16_t PressureDA_OutValue;           //PressureDAת�����ֵ
    uint32_t Pressure_ResultValue;          //Pressureԭʼֵ
}PressureDataConvert_Out_Param;




typedef struct
{
    BitAction FilterStart;                                                      //ÿ���˲���ʼ��־                                                           
    u8 Ms100_Cycle;                                                             //ÿ1s��õ��ݴ�������                        
    u32 EverySecCap;                                                            //ÿ��ĵ���ֵ                
    u32 FilArray[10];                                                           //ÿ������˲�����
}EverySecFilTypeDef;


#endif

