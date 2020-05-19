#include "BSP.h"

BitAction Add_Sub_Flag = Bit_RESET;
BitAction Calibrate_Flag = Bit_RESET;
extern BitAction Read_Add_Sub_FLag;
u16 Add_Sub_Cnt = 0;
u8 Pvd_Cnt = 0;  //PVD״̬�ϵ��ʱ
u8 pvd_temp[4];

extern u32 dwOilOutFifo[10]; 
extern u32 dwEepromOilValueMax;                                                        //���������������ֵ

u8 Start10s = 0;   //����10s��ʱ��־

void TIM3_IRQHandler(void)
{
    OS_ERR Err;
    OSIntEnter();   
    
    if (RESET != TIM_GetITStatus(TIM3, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        
        Pvd_Cnt++;
        
        if( (Pvd_Cnt > 50) && (RunVar.Pvd_Flag) )
        { 
          pvd_temp[0] = 0;
          RunVar.Pvd_Flag = 0;
          ProductPara.bFlashWEn = FLASH_WR_ENABLE;//Flashдʹ��
          I2C1_WriteNBytes(EEPROM_ADDRESS, PVD_FLAG, 1, pvd_temp );
          ProductPara.bFlashWEn = FLASH_WR_DISABLE;//Flashд��ֹ
        }  
          
        if(PCap_Res_Stau() & 0x100000)                                          //���PCap�Ĳɼ�״̬
        {
            RunVar.RawCap = PCap_Res_Value();                                   //��ȡ�ɼ�PCapֵ
            PCap_MEASURE();                                                     //����PCapģʽ        
            
            OSTaskSemPost(&AppFilterTCB, OS_OPT_POST_NONE, &Err);               //������Ϣ������
        }
        if(Read_Add_Sub_FLag)                                                   //���ݱ���1.6��(�ն˻��ڶ�ȡһ�����ݲ��ɹ�ʱ1�����ٴζ�ȡ��1.6s��֤�ܹ���ȡ2��)
        {
            if(++Add_Sub_Cnt >= 16)                                     
            {
                Read_Add_Sub_FLag = Bit_RESET;                                  
                Add_Sub_Flag = Bit_RESET;
                ProductPara.AddOil = 0;
                ProductPara.SubOil = 0;
            }
        }	
    }
    OSIntExit();   
}

void Timer3_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 999;                                    //TIM3��ʱ100ms�ɼ�PCapֵ
    TIM_TimeBaseInitStruct.TIM_Prescaler = 4799;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
    NVIC_Init(&NVIC_InitStruct);

    BSP_IntVectSet(BSP_INT_ID_TIM3, TIM3_IRQHandler);

    TIM_SetCounter(TIM3, 0);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void Pcap_INTN(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    
    BSP_IntVectSet(BSP_INT_ID_EXTI4_15, EXTI4_15_IRQHandler);
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource10);

    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Capture_TimerInit(void)
{
    Timer3_Init();
}



u16 CalcuFloaterDACode(u16 usRate, FloaterCalibTypeDef *pFcal)
{
    u32 i = 0;
    u32 head, tail;
    u32 u32Temp;

    for(i = 0; i < 8; i++)
    {
        if(usRate <= pFcal->Scale[i])
            break;
    }

    if(i == 0)  
    {
        return pFcal->DACode[0];
    }
    else if(i == 8) 
    {
        head = 6;
        tail = 7;
    }
    else
    {
        head = i - 1;
        tail = i;
    }

    /*
       usRate - pFcal->Scale[head]      pFcal->Scale[tail]  - pFcal->Scale[head]
      ---------------------------- = -------------------------------------------
        x - pFcal->DACode[head]         pFcal->DACode[tail] - pFcal->DACode[head]
       */

    u32Temp = (usRate - pFcal->Scale[head]);
    u32Temp *= (pFcal->DACode[tail] - pFcal->DACode[head]);
    u32Temp /= (pFcal->Scale[tail]  - pFcal->Scale[head]);
    u32Temp += pFcal->DACode[head];

    return((u16)u32Temp);
}

BitAction IsNeetResetPcap(void)
{
    static unsigned char ucRawCapEqualCnt = 0;
    BitAction bErrHappen = Bit_RESET;

    if(RunVar.RawCapBak == 0xffffffff)
    {
        RunVar.RawCapBak = RunVar.RawCap + 1;
    }

    if(RunVar.RawCapBak != RunVar.RawCap)
    {
        ucRawCapEqualCnt = 0;
    }
    else
    {
        ucRawCapEqualCnt += 1;
    }

    RunVar.RawCapBak = RunVar.RawCap;

    if(ucRawCapEqualCnt >= 10)
    {
        ucRawCapEqualCnt = 0;
        bErrHappen = Bit_SET;
    }
    return bErrHappen;
}


#define ADTIMEINTERVAL 100

u16 DAOutPutStabilised(u16 usRate, u16 usDACOutput)
{
    static u16 usBaseRate = 0xffff;
    static u16 usDABak;
    static u8 ucDirBak = 0;     //0��ʾ��ȣ�1��ʾ��С��2��ʾ����
    static u16 usLargeThan5PercCnt = 0;
    static u16 usLargeThan1PercCnt = 0;

    u16 usRateDelta;
    u8 ucDir;

    //��һ�����е�ʱ���ȡһ����������
    if (usBaseRate > 101)    //����usRate�Ǹ���ת��������������������1
    {
        usBaseRate = usRate;
        usDABak = usDACOutput;
        ucDirBak = 0;
        usLargeThan5PercCnt = 0;
        usLargeThan1PercCnt = 0;
        
        return usDABak;
    }

    if(usBaseRate == usRate)
    {
        ucDirBak = 0;
        usLargeThan5PercCnt = 0;
        usLargeThan1PercCnt = 0;
        
        return usDABak;
    }
    else if(usBaseRate > usRate)
    {
        ucDir = 1;
        usRateDelta = usBaseRate - usRate;
    }
    else
    {
        ucDir = 2;
        usRateDelta = usRate - usBaseRate;
    }

    //������ı䣬����ı������Ҳ����ı��жϵĻ�׼
    if(ucDir != ucDirBak)
    {
        ucDirBak = ucDir;
        usLargeThan5PercCnt = 0;
        usLargeThan1PercCnt = 0;
        
        return usDABak;
    }
    else
    {
        usLargeThan5PercCnt = (usRateDelta > 5) ? (usLargeThan5PercCnt + 1) : 0;

        usLargeThan1PercCnt = (usRateDelta > 1) ? (usLargeThan1PercCnt + 1) : 0;

        if((usLargeThan1PercCnt > (5000/ADTIMEINTERVAL)) || (usLargeThan5PercCnt > (6000/ADTIMEINTERVAL)))
        {
            usBaseRate = usRate;
            usDABak = usDACOutput;
            //ucDirBak = 0;
            usLargeThan5PercCnt = 0;
            usLargeThan1PercCnt = 0;
            
            return usDACOutput;
        }
        else
        {
            return usDABak;
        }
    }
}




/*
u8 ALGO_BubbleOrderFilter(u32 *pWDataBuf, u8 wLen, u8 wHeadLen, u8 wTailLen, u32 *pAverage)
{
  u8 i;
  u8 j;
  u32 wTemp;
  u32 u32FilterBuf[100];
    
  if (wLen < (wHeadLen + wTailLen))
  {
    return (0);
  }

  for(i = 0; i < wLen; i++)
  {
    u32FilterBuf[i] = *(pWDataBuf + i);
  }
  
  for (i = 0; i < wTailLen; i++)
  {
    for (j = 0; j < wLen - i - 1; j++)
    {
      if(u32FilterBuf[j] > u32FilterBuf[j + 1])
      {
        wTemp = u32FilterBuf[j];
        u32FilterBuf[j] = u32FilterBuf[j + 1];
        u32FilterBuf[j + 1] = wTemp;
      }
    }
  }

  for (i = 0; i < wHeadLen; i++)
  {
    for (j = wLen - wTailLen - 1; j > i; j--)
    {
      if (u32FilterBuf[j - 1] > u32FilterBuf[j])
      {
        wTemp = u32FilterBuf[j - 1];
        u32FilterBuf[j - 1] = u32FilterBuf[j];
        u32FilterBuf[j] = wTemp;
      }
    }
  }
    
  wTemp = 0;
  for(i = wHeadLen; i < wLen - wTailLen; i++)//dsdfsdf
  {
    wTemp += u32FilterBuf[i];
  }
    
  *pAverage = wTemp/(wLen - wHeadLen - wTailLen);
    
  return (1);
}
*/


u32 u60sFilter[60];                                                             //60���˲�ר��                
u32 u60sFilterBak[60];                                                          //60���˲���������                
EverySecFilTypeDef SecFilStr;

FlagStatus Get_EverySecPcap(void)                                               //���ÿ��ĵ���ֵ
{
    u8 i;
    
    RunVar.CapFromPCap = RunVar.RawCap;                                         //��ö�ȡ�ĵ���ֵ
  
    if(RunVar.CapFromPCap > ProductPara.CapMax)                                 //��ȡ�ĵ���ֵ�ڱ궨��������
    {
        RunVar.CapFromPCap = ProductPara.CapMax;
    }
    else if(RunVar.CapFromPCap < ProductPara.CapMin)
    {
        RunVar.CapFromPCap = ProductPara.CapMin;  
    }
    
    if(Calibrate_Flag == Bit_SET)                                               //�·��˱궨������������    ����궨������д��                     
    {
        SecFilStr.FilterStart = Bit_RESET;                                      //���˱궨����Ҫ���³�ʼ������ 
        Calibrate_Flag = Bit_RESET;
    }
    
    if(SecFilStr.FilterStart == Bit_RESET)                                      //��ʼ״̬��������                                   
    {
        SecFilStr.Ms100_Cycle = 0;
        SecFilStr.EverySecCap = 0;
        for(i = 0; i < sizeof(SecFilStr.FilArray)/sizeof(SecFilStr.FilArray[0]); i++)
        {
            SecFilStr.FilArray[i] = RunVar.CapFromPCap;
        }
        for(i = 0; i < sizeof(UserParam.HFil)/sizeof(UserParam.HFil[0]); i++)
        {
            UserParam.HFil[i] = RunVar.CapFromPCap;
        }
        for(i = 0; i < sizeof(UserParam.LFil)/sizeof(UserParam.LFil[0]); i++)
        {
            UserParam.LFil[i] = RunVar.CapFromPCap;
        }
        for(i = 0; i < 60; i++)
        {
            u60sFilter[i] = RunVar.CapFromPCap;
        }
        UserParam.PCap_Filter = RunVar.CapFromPCap;
        SecFilStr.FilterStart = Bit_SET;
        return RESET;
    }
    SecFilStr.FilArray[SecFilStr.Ms100_Cycle++] = RunVar.CapFromPCap;           //������
    if(SecFilStr.Ms100_Cycle >= 10)                                             //�����ˣ�ȥͷβ���ֵ
    {
        SecFilStr.EverySecCap = GetDelExtremeAndAverage(SecFilStr.FilArray,SecFilStr.Ms100_Cycle,SecFilStr.Ms100_Cycle/3,SecFilStr.Ms100_Cycle/3);
        SecFilStr.Ms100_Cycle = 0;
        return SET;                                                            //���ÿ��ĵ���ֵ
    }
    return RESET;
}



#include "string.h"
#define FILTER_LEN      3                                                       //������С���˷����ݸ���                                                   
//#define MULTIPLE        20                                                      //�����©������ϵ������
//#define CAL_LEN         FILTER_LEN * MULTIPLE                                   //1�����˲����飬��Ϊ10��
//#define CAL_GRO         CAL_LEN / 6                                             //1���ӷ�Ϊ10�飬ȥ������С��Ϊ��ʼֵ
//#define CAL_CHN         CAL_LEN / CAL_GRO

int xAxis[FILTER_LEN]={0,1,2};                                                  //ÿ��X���ֵ�������ͬ������ȡ�����ͬ��������ΪX��ֵ�Ϳ�����                    
int yAxis[FILTER_LEN];                                                          //ADֵY������

//u32 STA_END[CAL_GRO];                                                           //�Ѽ�©��֮ǰ/������ݲɼ�1���ӣ���10�Σ��ó���ʼ�ͽ���ֵ
//u32 CalSted[CAL_LEN];                                                           //�����©����ʼ�ͽ���ֵ����
//u32 CalStedBak[CAL_LEN];                                                      //�����©����ʼ�ͽ���ֵ���鱸��
//u32 CalStedBakChn[CAL_CHN];
BitAction InitArrayFlag = Bit_RESET;                                            //��ʼ�������־
BitAction Add_Finish_Flag = Bit_RESET;                                          //������ɱ�־        
BitAction Sub_Finish_Flag = Bit_RESET;                                          //©����ɱ�־
BitAction Add_Sub_Start_Flag = Bit_RESET;                                       //��©����ʼ��־                                     
u8 UpCnt,DoCnt,WaCnt,TimeCnt,FinishTimeCnt;                                     //Һλ�����½�ʱ�����
u32 MinAd,MaxAd,AddAd;                                                          
int OilAddValue,OilSubValue;


//��©���н�״̬
typedef enum
{
    IDLE = 0,
    ADD,
    SUB
}OilTypedef;


//��©����ʼ״̬
typedef enum
{
    STOP = 0,
    STRT,
}Add_SubTypedef;


OilTypedef OilState;
Add_SubTypedef AddState,SubState;
u8 LFilCnt;

//ȡ����ֵ������С���˷����õ���λ�ı仯�ʣ��ɴ��жϼ�©�����
void Judge_Add_Sub_Oil(u32 SecPCap)
{
    u8 i;
    u16 ADValue, AddLiqThr, SubLiqThr;
    u32 RealSecPCap;
    float Rate;
    int yK;
    //int BlindAreaLen;                                                         //ä������

    if(ProductPara.CompenEn == 1)
    {                                                                           //���ÿ�벹���ĵ���ֵ
        RealSecPCap = (u32)(SecPCap * UserParam.UserFlowK + UserParam.UserFlowB * 100 - 100);
    }
    else
    {
        RealSecPCap = SecPCap;                                                  //�������ĵ���ֵ
    }
                                                                                //����Һλռ�����̵İٷֱ�
    Rate = (RealSecPCap - ProductPara.CapMin) * 1.0f / ProductPara.CapRange;
    if(Rate > 1.0f)
    {
        Rate = 1.0f;
    }
    ADValue = (u32)(Rate * ProductPara.Range);                                  //�͸�������ΪAD�Ļ�׼ֵ(���̵�λ0.1mm)

    if(InitArrayFlag == Bit_RESET)
    {
        InitArrayFlag = Bit_SET;
        for(i = 0; i < FILTER_LEN; i++)
        {
            yAxis[i] = ADValue;
        }
        /*for(i = 0; i < CAL_LEN; i++)
        {
            CalSted[i] = ADValue;
        }*/
    }
 
    //memcpy((u8*)CalSted, (u8*)(CalSted + 1), (CAL_LEN - 1) * 4);
    //*(CalSted + CAL_LEN - 1) = ADValue;      

    memcpy((u8*)yAxis, (u8*)(yAxis+1), (FILTER_LEN-1) * 4);
    *(yAxis+FILTER_LEN-1) = ADValue;                                            //FIFO����Y��AD����   
    yK = get_slop(xAxis, yAxis, FILTER_LEN);                                    //��С���˷���������ֵ��б�ʣ���ÿ��Һλ���ߵĸ߶�*1000mm�� 
   
    ProductPara.BottomArea = (ProductPara.BoxPara[0] - 2 * ProductPara.BoxPara[3])\
                                * (ProductPara.BoxPara[1] - 2*ProductPara.BoxPara[3]) / 1000000.0f;//ƽ����
    ProductPara.AddMapHeight = ProductPara.AddLiqCnt * 100.0f / ProductPara.AddLiqTime / ProductPara.BottomArea;//���õ���ֵ��ӳ���ĸ߶�*1000mm
    ProductPara.SubMapHeight = ProductPara.SubLiqCnt * 100.0f / ProductPara.SubLiqTime / ProductPara.BottomArea;
    
    AddLiqThr = ProductPara.AddLiqCnt;            
    SubLiqThr = ProductPara.SubLiqCnt; 
    
    if(RunVar.AccStatus == 0x02)                                                //ACC����
    {
        if(RunVar.CarSpeed <= 5)                                                //�ж��ٶȣ�С�ڵ���5KM/H  
        {
            AddLiqThr = (u16)(1.5f * ProductPara.AddLiqCnt);                    //����������ֵΪ�趨������ֵ��1.5��
            SubLiqThr = (u16)(1.5f * ProductPara.SubLiqCnt);                    //�����½���ֵΪ�趨������ֵ��1.5��
        }
        else                                                                    //����5KM/H  22��
        {
            AddLiqThr = (u16)(2.2f * ProductPara.AddLiqCnt);                    //����5km/h��˵�������˶��������ֵ
            SubLiqThr = (u16)(2.2f * ProductPara.SubLiqCnt);
        } 
    }
    if(RunVar.CarSpeed > 8)
    {
        AddLiqThr = (u16)(2.2f * ProductPara.AddLiqCnt);                        //����5km/h��˵�������˶��������ֵ
        SubLiqThr = (u16)(2.2f * ProductPara.SubLiqCnt);        
    }
         
    if(++TimeCnt <= 20)                                                         //20����
    {
        if((yK > 0) && (yK > (u32)ProductPara.AddMapHeight))                    //б�ʴ���0�Ҵ�����ֵб��
        {
            UpCnt++;                                                            //Һλ��������+1
        }
        else if((yK < 0) && (abs(yK) > (u32)ProductPara.SubMapHeight))
        {
            DoCnt++;                                                            //Һλ�½�����+1
        }
        else                                                                    
        {
            WaCnt++;                                                            //��������                
        }
    }
    if(TimeCnt == 20)                                                           //20��һ������
    {
        TimeCnt = 0;
        if(UpCnt >= 8 * (DoCnt + WaCnt))                                        
        {
            if(OilState != ADD)
            {
                OilState = ADD;                                                 //���ڼ���
                if(AddState == STOP)
                {
                    SubState = STOP;
                    AddState = STRT;
                    if(Add_Sub_Start_Flag == Bit_RESET)                 
                    {
                        /*for(i = 0; i < CAL_GRO; i++)
                        {                                                       //�����ݽ����˲�                
                            memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                            STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
                        }
                        MinAd = Get_Min_Max(STA_END, CAL_GRO, 0);               //�����Сֵ��Ϊ�������  */
                        MinAd = RunVar.LiquidHeight;
                        MaxAd = 0;
                        Add_Sub_Start_Flag = Bit_SET;                           //��ʼ��©�ͱ�־��λ
                    }
                }
            }
        }
        else if(DoCnt >= 8 * (UpCnt + WaCnt))        
        {
            if(OilState != SUB)
            {
                OilState = SUB;                                                 //����©��
                if(SubState == STOP)
                {
                    AddState = STOP;
                    SubState = STRT;
                    if(Add_Sub_Start_Flag == Bit_RESET)
                    {
                        /*for(i = 0; i < CAL_GRO; i++)
                        {
                            memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                            STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
                        }
                        MaxAd = Get_Min_Max(STA_END, CAL_GRO, 1);*/
                        MaxAd = RunVar.LiquidHeight;
                        MinAd = 0;
                        Add_Sub_Start_Flag = Bit_SET;
                    }
                }       
            }
        }
        else
        {           
            OilState = IDLE;                                                    //��©�ͽ�������ֻ������
        }
        UpCnt = 0;
        DoCnt = 0;
        WaCnt = 0;                                                              //��0
        if(OilState == IDLE)                                                    //����ģʽ��
        {
            FinishTimeCnt++;
            if((AddState == STRT) && (Add_Finish_Flag == Bit_RESET))            //��⵽����
            {            
                /*emcpy((u8*)CalStedBak, (u8*)CalSted, FILTER_LEN * 10 * 4);
                MaxAd = GetDelExtremeAndAverage(CalStedBak, FILTER_LEN * 10,FILTER_LEN * 8, FILTER_LEN);
                BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.Range;
                if(BlindAreaLen <= 0)                                           //ä��С��0����ƽ̨�����������
                {
                    BlindAreaLen = 330;                         
                    //ä���̶�Ϊ33mm
                }       
                OilAddValue = Get_TankOil(MaxAd + BlindAreaLen) - Get_TankOil(MinAd + BlindAreaLen);
                if(OilAddValue > AddLiqThr)
                {
                    ProductPara.AddOil = OilAddValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }*/
                Add_Finish_Flag = Bit_SET;
                AddState = STOP;
            }
            if((SubState == STRT) && (Sub_Finish_Flag == Bit_RESET))            //��⵽©��
            {
                /*memcpy((u8*)CalStedBak, (u8*)CalSted, FILTER_LEN * 10 * 4);
                MinAd = GetDelExtremeAndAverage(CalStedBak, FILTER_LEN * 10, FILTER_LEN, FILTER_LEN * 8);
                BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.Range;
                if(BlindAreaLen <= 0)                                           //ä��С��0����ƽ̨�����������
                {
                    BlindAreaLen = 330;                                         //ä���̶�Ϊ33mm
                }   
                OilSubValue = Get_TankOil(MaxAd + BlindAreaLen) - Get_TankOil(MinAd + BlindAreaLen);
                if(OilSubValue > SubLiqThr)
                {
                    ProductPara.SubOil = OilSubValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }*/
                Sub_Finish_Flag = Bit_SET;
                SubState = STOP;              
            }
            //MaxAd = 0;
            //MinAd = 0;
        }
        else
        {
            FinishTimeCnt = 0;
            Add_Finish_Flag = Bit_RESET;
            Sub_Finish_Flag = Bit_RESET;
        }
    } 
    if(FinishTimeCnt >= 6)                                                      //�м�©��120s��
    {
        FinishTimeCnt = 0;
        if((Add_Finish_Flag == Bit_SET) || (Sub_Finish_Flag == Bit_SET))
        {
            /*for(i = 0; i < CAL_GRO; i++)
            {
                memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
            }*/
            //BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.SensorLen;
            /*if(BlindAreaLen <= 0)                                               //ä��С��0����ƽ̨�����������
            {
                BlindAreaLen = 330;                                             //ä���̶�Ϊ33mm
            }*/
            if(Add_Finish_Flag == Bit_SET)
            {
                Add_Finish_Flag = Bit_RESET;
                MaxAd = RunVar.LiquidHeight;                                          //�м��ͣ����Һλ����Ϊ�˲�ֵ
                //MaxAd = Get_Min_Max(STA_END, CAL_GRO, 1); 
       
                
               OilAddValue = Get_TankOil(MaxAd) - Get_TankOil(MinAd);
                
                
                
                
                
                if(OilAddValue > AddLiqThr)
                {
                    ProductPara.AddOil = OilAddValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }
            }
            else if(Sub_Finish_Flag == Bit_SET)
            {
                Sub_Finish_Flag = Bit_RESET;
                //MinAd = Get_Min_Max(STA_END, CAL_GRO, 0);
                MinAd = RunVar.LiquidHeight;                                    
                OilSubValue = Get_TankOil(MaxAd) - Get_TankOil(MinAd);
                if(OilSubValue > SubLiqThr)
                {
                    ProductPara.SubOil = OilSubValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }                    
            }
            MaxAd = 0;
            MinAd = 0;  
            Add_Sub_Start_Flag = Bit_RESET;                                     //��©�Ϳ�ʼ��־����
        }
    }
}





void DA_Handle(u16 LiquidAD, float rate)
{
    float smfTemp;
    
    if(ProductPara.LiquitHeightforDAMax > 0.1)
    {
        if(LiquidAD >= ProductPara.LiquitHeightforDAMax)
        {
            smfTemp = 1.0;
        }
        else
        {
            smfTemp = LiquidAD / ProductPara.LiquitHeightforDAMax;
        }
    }
    else
    {
        smfTemp = rate;
    }

    RunVar.DAForFloater = CalcuFloaterDACode((u16)(smfTemp * 100.0), &ProductPara.Floater);
    
    RunVar.DAForFloater = DAOutPutStabilised((u16)(smfTemp * 100.0), RunVar.DAForFloater);
    
    if(Bit_RESET == DAOilDebug.bDADebugEn)
    {
        DA_Write(RunVar.DAForFloater);
    }
    if(Bit_RESET == DAOutDebug.bDADebugEn)
    {
        DA2_Write((u16)(rate * (ProductPara.DAMax - ProductPara.DAMin) + ProductPara.DAMin));
    }    
}


u8 timecnt = 0;
u8 l_flag = 0;
u32 Curr_LiquidHeight = 0;
u32 Last_LiquidHeight = 0;

void Get_Filter_PCap(void)
{
    float rate;
    int difference;
    
    if(Get_EverySecPcap())                                                      //�õ�ÿ������
    {    
      
        if(UserParam.FilterLevel == 1)                                          //ʵʱ�˲�
        {
            UserParam.PCap_Filter = SecFilStr.EverySecCap;                      //�˲�ֵ��Ϊÿ���ֵ
        }
        else if(UserParam.FilterLevel == 2)                                     //ƽ���˲�60s����������
        {
            memcpy((u8*)u60sFilter, (u8*)(u60sFilter + 1), 236);
            *(u60sFilter+59) = SecFilStr.EverySecCap;
            //memset((u8*)u60sFilterBak, 0x00, 720);
            memcpy((u8*)u60sFilterBak, (u8*)u60sFilter, 240);
            UserParam.PCap_Filter = GetDelExtremeAndAverage(u60sFilterBak, 60, 20, 20);          
        }
        else                                                                    //ƽ����ƽ���˲�
        {                                                                       //���˲�ֵ����FIFO
            memcpy((u8*)UserParam.LFil, (u8*)(UserParam.LFil+1), (UserParam.FilterBufMax-1)*4);
            *(UserParam.LFil+UserParam.FilterBufMax-1) = SecFilStr.EverySecCap; //��ǰֵ�����β 
            if(++LFilCnt >= UserParam.FilterBufMax)                             //���˲���������
            {
                LFilCnt = 0;
                memcpy((u8*)UserParam.LFilBak, (u8*)UserParam.LFil, UserParam.FilterBufMax*4);//���˲�ֵ���뱸������
                memcpy((u8*)UserParam.HFil, (u8*)(UserParam.HFil+1), 36);       //���˲�ֵ����FIFO    ��ֵ�����β
                *(UserParam.HFil+9) = GetDelExtremeAndAverage(UserParam.LFilBak,UserParam.FilterBufMax,UserParam.FilterBufMax/3,UserParam.FilterBufMax/3);
                memcpy((u8*)UserParam.HFilBak, (u8*)UserParam.HFil, 40);        //���뱸������
                UserParam.PCap_Filter = GetDelExtremeAndAverage(UserParam.HFilBak,10,3,3);
            }
        }
        rate = (UserParam.PCap_Filter - ProductPara.CapMin) * 1.0f / ProductPara.CapRange;
        if(rate > 1.0f) rate = 1.0f;
        else if(rate < 0.00001) rate = 0;
        RunVar.LiquidAD = (u16)(rate * 65535);                                  //Һλ�߶�AD     
        RunVar.LiquidPercent = (u16)(rate * 1000.0f);                           //�����̵İٷֱ�
                  
        Curr_LiquidHeight = (u32)(rate * ProductPara.Range + ProductPara.BoxPara[2] * 10 - ProductPara.SensorLen + 200);//������ײ���Һλ�߶�    
        
        if(!l_flag)                                                             //�ϵ�ִ��һ�Σ����Һλ��ʼֵ
        {
            Last_LiquidHeight = Curr_LiquidHeight;                              //��ʼ��Һλ�߶�
            RunVar.LiquidHeight = Curr_LiquidHeight;
            l_flag = 1;
        }
        
        difference = Curr_LiquidHeight - Last_LiquidHeight;                     //��ǰ���ϴεĲ�ֵ        
        if(Add_Sub_Start_Flag)                                                  //��©��״̬ʱʵ��������
        {                     
            RunVar.LiquidHeight = Curr_LiquidHeight;                            //��ʼ��©��Һλ�߶�Ҫ����Ϊʵʱ�߶�        
            Last_LiquidHeight = Curr_LiquidHeight;
            timecnt = 0;
        }
        else                                                                    //�Ǽ�©��״̬
        {
            if(timecnt++ >= 15)                                                 //15sҺλ�仯һ��
            {
                timecnt = 0;
                if(abs(difference) > 1)                                         //Һλ�仯����0.1mm
                {
                    if(difference > 0)                                          //ֻ�仯0.1mm
                    {
                        RunVar.LiquidHeight += 1;
                    }
                    if(difference < 0)                                          //�½�0.4mm��ԭ���ǻ������������½���
                    {
                        if(difference < -4)
                        {
                            RunVar.LiquidHeight -= 4;
                        }
                        else
                        {
                            RunVar.LiquidHeight += difference;
                        }
                    }
                }        
                Last_LiquidHeight = RunVar.LiquidHeight;
            }
        }       
       
        
        if(Start10s < 10)
        {
           Start10s++;
           RunVar.OilQuantity = Get_TankOil(RunVar.LiquidHeight);                  //���Ҷ�Ӧ�����������������Ҫ���ڼ�©���ж�ǰ�棬��Ϊ��©���ж���Ҫ�õ����ֵ
           *(dwOilOutFifo+Start10s) = RunVar.OilQuantity;
        }
        else
        {     Start10s = 255;
              RunVar.OilQuantity = Get_TankOil(RunVar.LiquidHeight);                  //���Ҷ�Ӧ�����������������Ҫ���ڼ�©���ж�ǰ�棬��Ϊ��©���ж���Ҫ�õ����ֵ
              if(RunVar.OilQuantity <=  (u32)(dwEepromOilValueMax * 1.1) )   //�������궨ֵ��1.1��
            //20200511  �������ֵ���ڱ궨��������ֵʱ�����ǰһ�����������µ�ǰֵ��
            {     
                  memcpy((u8*)dwOilOutFifo, (u8*)(dwOilOutFifo + 1), 36);
                  *(dwOilOutFifo+9) = RunVar.OilQuantity;
               
                  RunVar.OilQuantity  =  GetDelExtremeAndAverage(dwOilOutFifo, 10, 3, 3);  
            }   
            else
           {
                  RunVar.OilQuantity  =  GetDelExtremeAndAverage(dwOilOutFifo, 10, 3, 3);
           }
        }       
        
        Judge_Add_Sub_Oil(SecFilStr.EverySecCap);                               //����Ϊÿ��Pcapֵ����ǰҺλ�߶�ֵ
        DA_Handle(RunVar.LiquidAD, rate);                                       //DA���
    }
}


//ȡ����ֵ������С���˷����õ���λ�ı仯�ʣ��ɴ��жϼ�©�����
void Judge_Fuel_Or_Leak(uint32_t PresureInputValue)
{
    u8 i;
    u16 ADValue, AddLiqThr, SubLiqThr;
    u32 RealSecPCap;
    float Rate;
    int yK;
    //int BlindAreaLen;                                                         //ä������

    if(ProductPara.CompenEn == 1)
    {                                                                           //���ÿ�벹���ĵ���ֵ
        RealSecPCap = (u32)(SecPCap * UserParam.UserFlowK + UserParam.UserFlowB * 100 - 100);
    }
    else
    {
        RealSecPCap = SecPCap;                                                  //�������ĵ���ֵ
    }
                                                                                //����Һλռ�����̵İٷֱ�
    Rate = (RealSecPCap - ProductPara.CapMin) * 1.0f / ProductPara.CapRange;
    if(Rate > 1.0f)
    {
        Rate = 1.0f;
    }
    ADValue = (u32)(Rate * ProductPara.Range);                                  //�͸�������ΪAD�Ļ�׼ֵ(���̵�λ0.1mm)

    if(InitArrayFlag == Bit_RESET)
    {
        InitArrayFlag = Bit_SET;
        for(i = 0; i < FILTER_LEN; i++)
        {
            yAxis[i] = ADValue;
        }
        /*for(i = 0; i < CAL_LEN; i++)
        {
            CalSted[i] = ADValue;
        }*/
    }
 
    //memcpy((u8*)CalSted, (u8*)(CalSted + 1), (CAL_LEN - 1) * 4);
    //*(CalSted + CAL_LEN - 1) = ADValue;      

    memcpy((u8*)yAxis, (u8*)(yAxis+1), (FILTER_LEN-1) * 4);
    *(yAxis+FILTER_LEN-1) = ADValue;                                            //FIFO����Y��AD����   
    yK = get_slop(xAxis, yAxis, FILTER_LEN);                                    //��С���˷���������ֵ��б�ʣ���ÿ��Һλ���ߵĸ߶�*1000mm�� 
   
    ProductPara.BottomArea = (ProductPara.BoxPara[0] - 2 * ProductPara.BoxPara[3])\
                                * (ProductPara.BoxPara[1] - 2*ProductPara.BoxPara[3]) / 1000000.0f;//ƽ����
    ProductPara.AddMapHeight = ProductPara.AddLiqCnt * 100.0f / ProductPara.AddLiqTime / ProductPara.BottomArea;//���õ���ֵ��ӳ���ĸ߶�*1000mm
    ProductPara.SubMapHeight = ProductPara.SubLiqCnt * 100.0f / ProductPara.SubLiqTime / ProductPara.BottomArea;
    
    AddLiqThr = ProductPara.AddLiqCnt;            
    SubLiqThr = ProductPara.SubLiqCnt; 
    
    if(RunVar.AccStatus == 0x02)                                                //ACC����
    {
        if(RunVar.CarSpeed <= 5)                                                //�ж��ٶȣ�С�ڵ���5KM/H  
        {
            AddLiqThr = (u16)(1.5f * ProductPara.AddLiqCnt);                    //����������ֵΪ�趨������ֵ��1.5��
            SubLiqThr = (u16)(1.5f * ProductPara.SubLiqCnt);                    //�����½���ֵΪ�趨������ֵ��1.5��
        }
        else                                                                    //����5KM/H  22��
        {
            AddLiqThr = (u16)(2.2f * ProductPara.AddLiqCnt);                    //����5km/h��˵�������˶��������ֵ
            SubLiqThr = (u16)(2.2f * ProductPara.SubLiqCnt);
        } 
    }
    if(RunVar.CarSpeed > 8)
    {
        AddLiqThr = (u16)(2.2f * ProductPara.AddLiqCnt);                        //����5km/h��˵�������˶��������ֵ
        SubLiqThr = (u16)(2.2f * ProductPara.SubLiqCnt);        
    }
         
    if(++TimeCnt <= 20)                                                         //20����
    {
        if((yK > 0) && (yK > (u32)ProductPara.AddMapHeight))                    //б�ʴ���0�Ҵ�����ֵб��
        {
            UpCnt++;                                                            //Һλ��������+1
        }
        else if((yK < 0) && (abs(yK) > (u32)ProductPara.SubMapHeight))
        {
            DoCnt++;                                                            //Һλ�½�����+1
        }
        else                                                                    
        {
            WaCnt++;                                                            //��������                
        }
    }
    if(TimeCnt == 20)                                                           //20��һ������
    {
        TimeCnt = 0;
        if(UpCnt >= 8 * (DoCnt + WaCnt))                                        
        {
            if(OilState != ADD)
            {
                OilState = ADD;                                                 //���ڼ���
                if(AddState == STOP)
                {
                    SubState = STOP;
                    AddState = STRT;
                    if(Add_Sub_Start_Flag == Bit_RESET)                 
                    {
                        /*for(i = 0; i < CAL_GRO; i++)
                        {                                                       //�����ݽ����˲�                
                            memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                            STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
                        }
                        MinAd = Get_Min_Max(STA_END, CAL_GRO, 0);               //�����Сֵ��Ϊ�������  */
                        MinAd = RunVar.LiquidHeight;
                        MaxAd = 0;
                        Add_Sub_Start_Flag = Bit_SET;                           //��ʼ��©�ͱ�־��λ
                    }
                }
            }
        }
        else if(DoCnt >= 8 * (UpCnt + WaCnt))        
        {
            if(OilState != SUB)
            {
                OilState = SUB;                                                 //����©��
                if(SubState == STOP)
                {
                    AddState = STOP;
                    SubState = STRT;
                    if(Add_Sub_Start_Flag == Bit_RESET)
                    {
                        /*for(i = 0; i < CAL_GRO; i++)
                        {
                            memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                            STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
                        }
                        MaxAd = Get_Min_Max(STA_END, CAL_GRO, 1);*/
                        MaxAd = RunVar.LiquidHeight;
                        MinAd = 0;
                        Add_Sub_Start_Flag = Bit_SET;
                    }
                }       
            }
        }
        else
        {           
            OilState = IDLE;                                                    //��©�ͽ�������ֻ������
        }
        UpCnt = 0;
        DoCnt = 0;
        WaCnt = 0;                                                              //��0
        if(OilState == IDLE)                                                    //����ģʽ��
        {
            FinishTimeCnt++;
            if((AddState == STRT) && (Add_Finish_Flag == Bit_RESET))            //��⵽����
            {            
                /*emcpy((u8*)CalStedBak, (u8*)CalSted, FILTER_LEN * 10 * 4);
                MaxAd = GetDelExtremeAndAverage(CalStedBak, FILTER_LEN * 10,FILTER_LEN * 8, FILTER_LEN);
                BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.Range;
                if(BlindAreaLen <= 0)                                           //ä��С��0����ƽ̨�����������
                {
                    BlindAreaLen = 330;                         
                    //ä���̶�Ϊ33mm
                }       
                OilAddValue = Get_TankOil(MaxAd + BlindAreaLen) - Get_TankOil(MinAd + BlindAreaLen);
                if(OilAddValue > AddLiqThr)
                {
                    ProductPara.AddOil = OilAddValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }*/
                Add_Finish_Flag = Bit_SET;
                AddState = STOP;
            }
            if((SubState == STRT) && (Sub_Finish_Flag == Bit_RESET))            //��⵽©��
            {
                /*memcpy((u8*)CalStedBak, (u8*)CalSted, FILTER_LEN * 10 * 4);
                MinAd = GetDelExtremeAndAverage(CalStedBak, FILTER_LEN * 10, FILTER_LEN, FILTER_LEN * 8);
                BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.Range;
                if(BlindAreaLen <= 0)                                           //ä��С��0����ƽ̨�����������
                {
                    BlindAreaLen = 330;                                         //ä���̶�Ϊ33mm
                }   
                OilSubValue = Get_TankOil(MaxAd + BlindAreaLen) - Get_TankOil(MinAd + BlindAreaLen);
                if(OilSubValue > SubLiqThr)
                {
                    ProductPara.SubOil = OilSubValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }*/
                Sub_Finish_Flag = Bit_SET;
                SubState = STOP;              
            }
            //MaxAd = 0;
            //MinAd = 0;
        }
        else
        {
            FinishTimeCnt = 0;
            Add_Finish_Flag = Bit_RESET;
            Sub_Finish_Flag = Bit_RESET;
        }
    } 
    if(FinishTimeCnt >= 6)                                                      //�м�©��120s��
    {
        FinishTimeCnt = 0;
        if((Add_Finish_Flag == Bit_SET) || (Sub_Finish_Flag == Bit_SET))
        {
            /*for(i = 0; i < CAL_GRO; i++)
            {
                memcpy((u8*)CalStedBakChn, (u8*)(CalSted + CAL_CHN * i), CAL_CHN * 4);
                STA_END[i] = GetDelExtremeAndAverage(CalStedBakChn, CAL_CHN, CAL_CHN / 3, CAL_CHN / 3);
            }*/
            //BlindAreaLen = ProductPara.BoxPara[2] * 10 + 200 - ProductPara.SensorLen;
            /*if(BlindAreaLen <= 0)                                               //ä��С��0����ƽ̨�����������
            {
                BlindAreaLen = 330;                                             //ä���̶�Ϊ33mm
            }*/
            if(Add_Finish_Flag == Bit_SET)
            {
                Add_Finish_Flag = Bit_RESET;
                MaxAd = RunVar.LiquidHeight;                                          //�м��ͣ����Һλ����Ϊ�˲�ֵ
                //MaxAd = Get_Min_Max(STA_END, CAL_GRO, 1); 
       
                
               OilAddValue = Get_TankOil(MaxAd) - Get_TankOil(MinAd);
                
                
                
                
                
                if(OilAddValue > AddLiqThr)
                {
                    ProductPara.AddOil = OilAddValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }
            }
            else if(Sub_Finish_Flag == Bit_SET)
            {
                Sub_Finish_Flag = Bit_RESET;
                //MinAd = Get_Min_Max(STA_END, CAL_GRO, 0);
                MinAd = RunVar.LiquidHeight;                                    
                OilSubValue = Get_TankOil(MaxAd) - Get_TankOil(MinAd);
                if(OilSubValue > SubLiqThr)
                {
                    ProductPara.SubOil = OilSubValue;                           //����ֵ
                    Add_Sub_Flag = Bit_SET;                                     //��©�ͱ�־
                }                    
            }
            MaxAd = 0;
            MinAd = 0;  
            Add_Sub_Start_Flag = Bit_RESET;                                     //��©�Ϳ�ʼ��־����
        }
    }
}





/**
* ����       : Pressure_Levle_DataConvertFunc()
* ����       : 2020-05-16 
* �ο���     ��ԭ����Һλ��������غ�����filter.c-->void Get_Filter_PCap(void)��
* �༭����   ��LEE  
* ����       : ѹ������ת����ʵ��������� 
* �������   : DataConvert_Param,ָ�����Ʋ����ṹ��;PresureInputValue,���������
* �������   : DataConvert_Out,������ݵĲ����ṹ��
* ���ؽ��   : ��
* ע���˵�� : ʹ�ñ�����ǰ,�Ƚ�DataConvert_Param��ʼ��(��������ÿһ����Ա�����ʵ�ֵ),����������ʹ��
* �޸�����   : 
*/
void Pressure_Level_DataConvertFunc(PressureDataConvert_Param *DataConvert_Param, P_UserParamTypeDef *UserParam, 
                                uint32_t PresureInputValue, 
                                PressureDataConvert_Out_Param *DataConvert_Out)
{
    uint16_t PressureAD;                //ѹ��ADֵ��ʱ����
    uint32_t ResultTemp;               //ѹ��ֵ��ʱ����
    float rate;                         //ѹ��ֵ����ֵ��ʱ����
    
    
    /*
    //����ʹ��,����K,Bֵ
    if(DataConvert_Param->CompenEn == COMPENSATE_ENABLE)        
    {
        ResultTemp = (uint32_t)(PresureInputValue * DataConvert_Param->Correct_K 
                                + (DataConvert_Param->Correct_B - 100));
    }
    else
    {
        ResultTemp = PresureInputValue;
    }
    //���㵱ǰѹ��ֵ�� ������ѹ���ı���
    PressureRateTemp = (float)(ResultTemp - DataConvert_Param->PressureMin) 
                                / (float)(DataConvert_Param->PressureMax - DataConvert_Param->PressureMin);
    
    if(1.0 < PressureRateTemp)
    {
        PressureRateTemp = 1.0;
    }
    else if (0.0 > PressureRateTemp)
    {
        PressureRateTemp = 0.0;
    }
    //ת����0--65535��Χ�ڵ�ADֵ
    PressureAD = (uint16_t)(PressureRateTemp * PCAP_ADC_MAX_VALUE);
  */
    
        int difference;

        if(UserParam->FilterLevel == 1)                                          //ʵʱ�˲�
        {
            UserParam->Pressure_Filter = PresureInputValue;                      //�˲�ֵ��Ϊÿ���ֵ
        }
        else if(UserParam->FilterLevel == 2)                                     //ƽ���˲�60s����������
        {
            memcpy((u8*)u60sFilter, (u8*)(u60sFilter + 1), 236);
            *(u60sFilter+59) = PresureInputValue;
            //memset((u8*)u60sFilterBak, 0x00, 720);
            memcpy((u8*)u60sFilterBak, (u8*)u60sFilter, 240);
            UserParam->Pressure_Filter = GetDelExtremeAndAverage(u60sFilterBak, 60, 20, 20);          
        }
        else                                                                    //ƽ����ƽ���˲�
        {                                                                       //���˲�ֵ����FIFO
            memcpy((u8*)UserParam->LFil, (u8*)(UserParam->LFil+1), (UserParam->FilterBufMax-1)*4);
            *(UserParam->LFil + UserParam->FilterBufMax-1) = PresureInputValue; //��ǰֵ�����β 
            if(++LFilCnt >= UserParam->FilterBufMax)                             //���˲���������
            {
                LFilCnt = 0;
                memcpy((u8*)UserParam->LFilBak, (u8*)UserParam->LFil, UserParam->FilterBufMax*4);//���˲�ֵ���뱸������
                memcpy((u8*)UserParam->HFil, (u8*)(UserParam->HFil+1), 36);       //���˲�ֵ����FIFO    ��ֵ�����β
                *(UserParam->HFil+9) = GetDelExtremeAndAverage(UserParam->LFilBak,UserParam->FilterBufMax,UserParam->FilterBufMax/3,UserParam->FilterBufMax/3);
                memcpy((u8*)UserParam->HFilBak, (u8*)UserParam->HFil, 40);        //���뱸������
                UserParam->Pressure_Filter = GetDelExtremeAndAverage(UserParam->HFilBak,10,3,3);
            }
        }
        rate = (UserParam->Pressure_Filter - ProductPara.CapMin) * 1.0f / ProductPara.CapRange;
        if(rate > 1.0f) rate = 1.0f;
        else if(rate < 0.00001) rate = 0;
        DataConvert_Out->LiquidHeightAD = (u16)(rate * 65535);                                  //Һλ�߶�AD     
        DataConvert_Out->LiquidPercent  = (u16)(rate * 1000.0f);                           //�����̵İٷֱ�
                  
        Curr_LiquidHeight = (u32)(rate * ProductPara.Range + ProductPara.BoxPara[2] * 10 - ProductPara.SensorLen + 200);//������ײ���Һλ�߶�    
        
        if(!l_flag)                                                             //�ϵ�ִ��һ�Σ����Һλ��ʼֵ
        {
            Last_LiquidHeight = Curr_LiquidHeight;                              //��ʼ��Һλ�߶�
            DataConvert_Out->LiquidHeight = Curr_LiquidHeight;
            l_flag = 1;
        }
        
        difference = Curr_LiquidHeight - Last_LiquidHeight;                     //��ǰ���ϴεĲ�ֵ        
        if(Add_Sub_Start_Flag)                                                  //��©��״̬ʱʵ��������
        {                     
            DataConvert_Out->LiquidHeight = Curr_LiquidHeight;                            //��ʼ��©��Һλ�߶�Ҫ����Ϊʵʱ�߶�        
            Last_LiquidHeight = Curr_LiquidHeight;
            timecnt = 0;
        }
        else                                                                    //�Ǽ�©��״̬
        {
            if(timecnt++ >= 15)                                                 //15sҺλ�仯һ��
            {
                timecnt = 0;
                if(abs(difference) > 1)                                         //Һλ�仯����0.1mm
                {
                    if(difference > 0)                                          //ֻ�仯0.1mm
                    {
                        DataConvert_Out->LiquidHeight += 1;
                    }
                    if(difference < 0)                                          //�½�0.4mm��ԭ���ǻ������������½���
                    {
                        if(difference < -4)
                        {
                            DataConvert_Out->LiquidHeight -= 4;
                        }
                        else
                        {
                            DataConvert_Out->LiquidHeight += difference;
                        }
                    }
                }        
                Last_LiquidHeight = DataConvert_Out->LiquidHeight;
            }
        }       
       
      ////////////////////////////////////////////////////////��������  
//        if(Start10s < 10)
//        {
//           Start10s++;
//           RunVar.OilQuantity = Get_TankOil(RunVar.LiquidHeight);                  //���Ҷ�Ӧ�����������������Ҫ���ڼ�©���ж�ǰ�棬��Ϊ��©���ж���Ҫ�õ����ֵ
//           *(dwOilOutFifo+Start10s) = RunVar.OilQuantity;
//        }
//        else
//        {     Start10s = 255;
//              RunVar.OilQuantity = Get_TankOil(RunVar.LiquidHeight);                  //���Ҷ�Ӧ�����������������Ҫ���ڼ�©���ж�ǰ�棬��Ϊ��©���ж���Ҫ�õ����ֵ
//              if(RunVar.OilQuantity <=  (u32)(dwEepromOilValueMax * 1.1) )   //�������궨ֵ��1.1��
//            //20200511  �������ֵ���ڱ궨��������ֵʱ�����ǰһ�����������µ�ǰֵ��
//            {     
//                  memcpy((u8*)dwOilOutFifo, (u8*)(dwOilOutFifo + 1), 36);
//                  *(dwOilOutFifo+9) = RunVar.OilQuantity;
//               
//                  RunVar.OilQuantity  =  GetDelExtremeAndAverage(dwOilOutFifo, 10, 3, 3);  
//            }   
//            else
//           {
//                  RunVar.OilQuantity  =  GetDelExtremeAndAverage(dwOilOutFifo, 10, 3, 3);
//           }
//        }       
       //////////////////////////////////////////////////////////////////
       
        //Judge_Add_Sub_Oil(SecFilStr.EverySecCap);                               //����Ϊÿ��Pcapֵ����ǰҺλ�߶�ֵ
        DA_Handle(DataConvert_Out->LiquidHeightAD, rate);                                       //DA���   
}




void App_Filter_Task (void *p_arg)
{
    OS_ERR Err; 
      
    BitAction bNeetResetPcap = Bit_RESET;
    Capture_TimerInit();
    DA_Init();
    Last_LiquidHeight = Curr_LiquidHeight;
    
    while(1)
    {
#if IWDG_EN > 0
        IWDG_ReloadCounter();
#endif
        OSTaskSemPend(0, OS_OPT_PEND_BLOCKING, NULL, &Err);
        if (Err == OS_ERR_NONE)
        {
            bNeetResetPcap = IsNeetResetPcap();                                 //�Ƿ���Ҫ����PCap
            
            if(Bit_SET == bNeetResetPcap)
            {
                bNeetResetPcap = Bit_RESET;
                resetPcap();                                                    //����
                RunVar_Init();
                Capture_TimerInit();
                continue;
            }
            Get_Filter_PCap();                                
        }
        
    }
}

