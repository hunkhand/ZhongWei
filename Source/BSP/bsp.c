
/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM32F0518-EVAL
*                                          Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : FF
*                 MD
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   BSP_MODULE
#include  <bsp.h>

#if IWDG_EN > 0
void IWDG_Init(void)
{
    /*!< LSI Enable */
    RCC_LSICmd(ENABLE);

    /*!< Wait till LSI is ready */

    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {}

    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescaler(IWDG_Prescaler_64);   // 40K / 64 = 625 -> 1.6ms per tick

    /* Set counter reload value*/

    IWDG_SetReload(125);         // 1.6ms * 125 = 200ms

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

#endif

void RunVar_Init(void)
{
    RunVar.CalState = Def_State;
    RunVar.RawCapBak = 0xffffffff;
    RunVar.HeightBak = 500000.5;
}

    //2）、当工作电压低于设定阀值时，将产生PVD中断，在中断程序中进行相应的处理：
void PVD_IRQHandler(void)
{
    uint8_t i;
    u8 opara[4],pvd_temp[4] ;
    
    if( (EXTI->PR) == 0x00010000)// (PWR_FLAG_PVDO)!=RESET)
    {
      
      ProductPara.bFlashWEn = FLASH_WR_ENABLE;//Flash写使能
        
      pvd_temp[0] = 1;
      I2C1_WriteNBytes(EEPROM_ADDRESS, PVD_FLAG, 1,  pvd_temp );  //写PVD标志

      
      
      for(i = 4; i > 0; i--)
        {
          opara[4 - i] = (u8)(RunVar.LiquidHeight >> ((i - 1) * 8));
          opara[4 - i] = (u8)(RunVar.LiquidHeight >> ((i - 1) * 8));
          
        }     
     
       I2C1_WriteNBytes(EEPROM_ADDRESS, PVD_LIQUIDHEIGHT, 4, opara);
           
      //RunVar.OilQuantity = 2000;
       for(i = 4; i > 0; i--)
        {
          opara[4 - i] = (u8)(RunVar.OilQuantity >> ((i - 1) * 8));
        }     
     
       I2C1_WriteNBytes(EEPROM_ADDRESS, PVD_OILQUANTITY, 4, opara);
          
     }
     ProductPara.bFlashWEn = FLASH_WR_DISABLE;//Flash写禁止
     EXTI_ClearITPendingBit(EXTI_Line16);   //退出前清中断标志位      
 }


void PVD_Config(void)
{
        NVIC_InitTypeDef NVIC_InitStructure;
        EXTI_InitTypeDef EXTI_InitStructure;
        
        BSP_IntVectSet(BSP_INT_ID_PVD, PVD_IRQHandler);
        
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        
       // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
        NVIC_InitStructure.NVIC_IRQChannel = PVD_VDDIO2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
       // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        
        NVIC_Init(&NVIC_InitStructure);
        
        //EXTI_StructInit(&EXTI_InitStructure);
        //EXTI_ClearITPendingBit(EXTI_Line16);
        EXTI_InitStructure.EXTI_Line = EXTI_Line16;        //ía2??D????16×¨?aPVD?D??ê1ó?
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //EXTI_Trigger_Rising_Falling;        //μ??1μíóú·§?μê±2úéú?D??
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;       //ê1?ü

        EXTI_Init(&EXTI_InitStructure);
        
        PWR_PVDLevelConfig(PWR_PVDLevel_7);
        PWR_PVDCmd(ENABLE);
}


//1）、系统启动后启动PVD，并开启相应的中断。
//void PWR_PVD_Config(void)
//{
//    EXTI_InitTypeDef EXTI_InitStructure;
//    PWR_PVDLevelConfig(PWR_PVDLevel_7); // 设定监控阀值
//    
//    PWR_PVDCmd(ENABLE);// 使能PVD
//    
//    EXTI_StructInit(&EXTI_InitStructure);
//    EXTI_InitStructure.EXTI_Line = EXTI_Line16; // PVD连接到中断线16上
//    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //使用中断模式
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//EXTI_Trigger_Raising;//电压低于阀值时产生中断
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE; // 使能中断线
//    EXTI_Init(&EXTI_InitStructure); // 初始
////    EXTI_InitStructure.EXTI_Trigger的赋值可选项：
////    EXTI_Trigger_Rising---表示电压从高下降到低于设定阀值时产生中断；
////    EXTI_Trigger_Falling---表示电压从低上升到高于设定阀值时产生中断；
////    EXTI_Trigger_Rising_Falling---表示电压上升或下降越过设定阀值时都产生中断。
// 
//}
//
//void NVIC_Configuration(void)
//{ 
//        NVIC_InitTypeDef NVIC_InitStructure;
//        
//        /*使能 PWR 时钟 */
//        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
//          /* enabling EXTI interrupt */
//     //   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//        NVIC_InitStructure.NVIC_IRQChannel=PVD_VDDIO2_IRQn;// PVD_IRQn;
//        NVIC_InitStructure.NVIC_IRQChannelPriority=0;
//        //NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
//        NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;
//        NVIC_Init(&NVIC_InitStructure);
//}
//      


/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*
*               (2) CPU instruction / data tracing requires the use of the following pins :
*                   (a) (1) Aysynchronous     :  PB[3]
*                       (2) Synchronous 1-bit :  PE[3:2]
*                       (3) Synchronous 2-bit :  PE[4:2]
*                       (4) Synchronous 4-bit :  PE[6:2]
*
*                   (c) The application may wish to adjust the trace bus width depending on I/O
*                       requirements.
*********************************************************************************************************
*/

void  BSP_Init(void)
{
    BSP_IntInit();

    EEProm_Init();
    
    LED_Init();
    Rd_Dev_Param();
    PCap_Init();
    RunVar_Init();
    
    PVD_Config();
    
    //PWR_PVD_Config();
    //NVIC_Configuration();
#if IWDG_EN > 0
    IWDG_Init();
#endif
}

/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq(void)
{

    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);
    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR'
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR'
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time
*                       but MUST be less than the maximum measured time; otherwise, timer resolution
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit(void)
{

}

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_Get32(),
*               CPU_TS_Get64(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR'
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR'
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer period            Timer's period in some units of
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same
*                                                                   units of (fractional) seconds
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer frequency         Timer's frequency in some units
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less
*                           than the maximum measured time; otherwise, timer resolution inadequate to
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd(void)
{
    CPU_TS_TMR  ts_tmr_cnts;


    ts_tmr_cnts = 0u;

    return (ts_tmr_cnts);
}

#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                         CPU_TSxx_to_uSec()
*
* Description : Convert a 32-/64-bit CPU timestamp from timer counts to microseconds.
*
* Argument(s) : ts_cnts   CPU timestamp (in timestamp timer counts [see Note #2aA]).
*
* Return(s)   : Converted CPU timestamp (in microseconds           [see Note #2aD]).
*
* Caller(s)   : Application.
*
*               This function is an (optional) CPU module application programming interface (API)
*               function which MAY be implemented by application/BSP function(s) [see Note #1] &
*               MAY be called by application function(s).
*
* Note(s)     : (1) CPU_TS32_to_uSec()/CPU_TS64_to_uSec() are application/BSP functions that MAY be
*                   optionally defined by the developer when either of the following CPU features is
*                   enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) The amount of time measured by CPU timestamps is calculated by either of
*                       the following equations :
*
*                                                                        10^6 microseconds
*                       (1) Time measured  =   Number timer counts   *  -------------------  *  Timer period
*                                                                            1 second
*
*                                              Number timer counts       10^6 microseconds
*                       (2) Time measured  =  ---------------------  *  -------------------
*                                                Timer frequency             1 second
*
*                               where
*
*                                   (A) Number timer counts     Number of timer counts measured
*                                   (B) Timer frequency         Timer's frequency in some units
*                                                                   of counts per second
*                                   (C) Timer period            Timer's period in some units of
*                                                                   (fractional)  seconds
*                                   (D) Time measured           Amount of time measured,
*                                                                   in microseconds
*
*                   (b) Timer period SHOULD be less than the typical measured time but MUST be less
*                       than the maximum measured time; otherwise, timer resolution inadequate to
*                       measure desired times.
*
*                   (c) Specific implementations may convert any number of CPU_TS32 or CPU_TS64 bits
*                       -- up to 32 or 64, respectively -- into microseconds.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_32_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS32_to_uSec(CPU_TS32  ts_cnts)                 /* 32-bit conversion                                    */
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;


    fclk_freq = BSP_CPU_ClkFreq();
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}

#endif


#if (CPU_CFG_TS_64_EN == DEF_ENABLED)
CPU_INT64U  CPU_TS64_to_uSec(CPU_TS64  ts_cnts)                 /* 64-bit conversion                                    */
{
    CPU_INT64U  ts_us;
    CPU_INT64U  fclk_freq;


    fclk_freq = BSP_CPU_ClkFreq();
    ts_us     = ts_cnts / (fclk_freq / DEF_TIME_NBR_uS_PER_SEC);

    return (ts_us);
}

#endif
