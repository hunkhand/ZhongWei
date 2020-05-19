#include "BSP.h"



#define AD_GROUP_MAX 40
#define AD_CHANNEL_MAX 2
#define AD_DELETE_NUM  10

#define AD_FILTER_MAX 30
#define AD_FILTER_DELETE_NUM 10


vu16  usADC1ConvertedValue[AD_GROUP_MAX][AD_CHANNEL_MAX];
u16  usADC1Value[AD_CHANNEL_MAX][AD_GROUP_MAX];

//extern u32 g_cyTankOil;

typedef struct
{
    unsigned long   FilterIndex;
    unsigned short  usFilterBuf[AD_FILTER_MAX];
}struct_ADCFilter;

struct_ADCFilter  ADC1Filter[AD_CHANNEL_MAX];

unsigned short  usFilterBuf[AD_FILTER_MAX];

static void ADC_Config(void);
static void DMA_Config(void);

/**
  * @brief  ADC1 channel12 configuration
  * @param  None
  * @retval None
  */
static void ADC_Config(void)
{
    ADC_InitTypeDef       ADC_InitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;

    /* ADC1 DeInit */
    ADC_DeInit(ADC1);

    /* Enable ADC and GPIO clocks ****************************************/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);

    /* Configure ADC1 Channel12 pin as analog input ******************************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Initialize ADC structure */
    ADC_StructInit(&ADC_InitStructure);

    /* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* Convert the ADC1 Channel 12 with 239.5 Cycles as sampling time */
    ADC_ChannelConfig(ADC1, ADC_Channel_6 , ADC_SampleTime_239_5Cycles);
    ADC_ChannelConfig(ADC1, ADC_Channel_7 , ADC_SampleTime_239_5Cycles);

    /* ADC Calibration */
    ADC_GetCalibrationFactor(ADC1);

    /* Enable DMA request after last transfer (OneShot-ADC mode) */
    ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);  //????DMA?-?¡¤?¡ê¨º?

    /* Enable ADCperipheral[PerIdx] */
    ADC_Cmd(ADC1, ENABLE);

    /* Enable ADC_DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Wait the ADCEN falg */

    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY));

    /* ADC1 regular Software Start Conv */
    ADC_StartOfConversion(ADC1);
}

/**
  * @brief  DMA channel1 configuration
  * @param  None
  * @retval None
  */
static void DMA_Config(void)
{
    DMA_InitTypeDef  DMA_InitStructure;

    /* Enable DMA1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* DMA1 Stream1 channel1 configuration **************************************/
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) & usADC1ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = AD_GROUP_MAX * AD_CHANNEL_MAX;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

void Temperature_Init(void)
{
    ADC_Config();
    DMA_Config();
}

void InitTmpQueue(struct_ADCFilter *pADCDF, unsigned short uidata)
{
    for(unsigned long i = 0; i < AD_FILTER_MAX; i++)
    {
        pADCDF->usFilterBuf[i] = uidata;
    }
}

/****/
ANGLE_TypeDef Angle;

void EXTI4_15_IRQHandler(void)
{
    OS_ERR  Err;
    OSIntEnter();   
	
    if(RESET != EXTI_GetITStatus(EXTI_Line12))
    {
        EXTI_ClearITPendingBit(EXTI_Line12);

        OSTaskSemPost(&AppMEMSTCB, OS_OPT_POST_NONE, &Err);
    }
    else if(RESET != EXTI_GetITStatus(EXTI_Line10))
    {
        EXTI_ClearITPendingBit(EXTI_Line10);
        PCap_Ready = Bit_SET;
    }

    OSIntExit();  
}

void MMA8652_INTN_Init(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    BSP_IntVectSet(BSP_INT_ID_EXTI4_15, EXTI4_15_IRQHandler);
    
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Connect EXTI12 Line to PB12 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource12);

    /* Configure EXTI12line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI4_15 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void MMA8652_Init(void)
{
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();

	MMA8652_INTN_Init();
	
	MMA8652_GPIO_Config();     
	
	MMA8652_Setting();
	
	CPU_CRITICAL_EXIT();
}

void App_MEMSAndTemp_Task (void *p_arg)
{

    	BitAction bTempInitate;
	OS_ERR Err;
	float ftemp;
	unsigned short usTemp;
    
#if MMA8652
	ACCEL_TypeDef Accel;
#endif

    bTempInitate = Bit_RESET;
	
#if MMA8652	
	MMA8652_Init();
#endif

	Temperature_Init();
	ADC1Filter[0].FilterIndex = 0;
        ADC1Filter[1].FilterIndex = 0;
	
	while(1)
	{
			#if IWDG_EN > 0
					IWDG_ReloadCounter();
			#endif	

			//OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_DLY, &Err);
			
			//g_cyTankOil = Get_TankOil(RunVar.LiquidHeightValue);
		OSTaskSemPend(1000, OS_OPT_PEND_BLOCKING, NULL, &Err);
		
#if MMA8652
		
		if(OS_ERR_TIMEOUT == Err)
		{
			MMA8652_Init();
		}
		Multiple_read_MMA8652(&Accel);
		
		getAngle(&Accel, &Angle);
#endif		

		
		CPU_SR_ALLOC();
        CPU_CRITICAL_ENTER();
        for(long i = 0; i < AD_GROUP_MAX; i++)
        {
            for(long j = 0; j < AD_CHANNEL_MAX; j++)
            {
			    usADC1Value[j][i]=usADC1ConvertedValue[i][j];
            }
        }
        CPU_CRITICAL_EXIT();
        
        //usTemp = GetDelExtremeAndAverage(usADC1Value[0], AD_GROUP_MAX, AD_DELETE_NUM, AD_DELETE_NUM);

        if(Bit_SET == bTempInitate)
        {
            InitTmpQueue(&ADC1Filter[0], usTemp);
        }
        
        ADC1Filter[0].usFilterBuf[ADC1Filter[0].FilterIndex] = usTemp;
        if (++ADC1Filter[0].FilterIndex >= AD_FILTER_MAX)
            ADC1Filter[0].FilterIndex = 0;

        for (long j = 0; j < AD_FILTER_MAX; j++)
        {
            usFilterBuf[j] = ADC1Filter[0].usFilterBuf[j];
        }

        //ftemp = GetDelExtremeAndAverage(usFilterBuf, AD_FILTER_MAX, AD_FILTER_DELETE_NUM, AD_FILTER_DELETE_NUM);

        ftemp = ftemp * 3.3 / 4095;
        ftemp = (ftemp - 0.5) * 100;
        
        RunVar.TempInSensor = ftemp * ProductPara.Temp_K + ProductPara.Temp_B;


        //usTemp = GetDelExtremeAndAverage(usADC1Value[1], AD_GROUP_MAX, AD_DELETE_NUM, AD_DELETE_NUM);

        if(Bit_SET == bTempInitate)
        {
            InitTmpQueue(&ADC1Filter[1], usTemp);
        }
        
        ADC1Filter[1].usFilterBuf[ADC1Filter[1].FilterIndex] = usTemp;
        if (++ADC1Filter[1].FilterIndex >= AD_FILTER_MAX)
            ADC1Filter[1].FilterIndex = 0;

        for (long j = 0; j < AD_FILTER_MAX; j++)
        {
            usFilterBuf[j] = ADC1Filter[1].usFilterBuf[j];
        }

        //ftemp = GetDelExtremeAndAverage(usFilterBuf, AD_FILTER_MAX, AD_FILTER_DELETE_NUM, AD_FILTER_DELETE_NUM);

        ftemp = ftemp * 3.3 / 4095;
        ftemp = (ftemp - 0.5) * 100;
        RunVar.TempInAir= ftemp * ProductPara.Temp2_K + ProductPara.Temp2_B;

        bTempInitate = Bit_SET;
		
	}
}



