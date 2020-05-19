/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                         EXCEPTION VECTORS
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                           STM32F0518-EVAL
*                                          Evaluation Board
*
* Filename      : cstartup.c
* Version       : V1.00
* Programmer(s) : FT
*                 FF
*                 MD
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef  union
{
    CPU_FNCT_VOID   Fnct;
    void           *Ptr;
} APP_INTVECT_ELEM;


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#pragma language=extended
#pragma segment="CSTACK"

static  void  App_NMI_ISR         (void);

static  void  App_Fault_ISR       (void);

static  void  App_Spurious_ISR    (void);

static  void  App_Reserved_ISR    (void);

extern  void  __iar_program_start (void);

extern  void  TIM7_IRQHandler     (void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                  EXCEPTION / INTERRUPT VECTOR TABLE
*
* Note(s) : (1) 32 maskable interrupt channels (not including the sixteen Cortex-M0 interrupt lines).
*
*           (2) Interrupts vector 2-13 are implemented in this file as infinite loop for debuging
*               purposes only. The application might implement a recover procedure if it is needed.
*
*           (3) OS_CPU_PendSVHandler() and OS_CPU_SysTickHandler() are implemented in the generic OS
*               port.
*
*           (4) Interrupts 18, 29 and 31 are Reserved.
*
*********************************************************************************************************
*/

__root  const  APP_INTVECT_ELEM  __vector_table[] @ ".intvec" =
{
    { .Ptr = (void *)__sfe("CSTACK")},                          /*  0, SP start value.                                      */
             __iar_program_start,                                        /*  1, PC start value.                                      */
             App_NMI_ISR,                                                /*  2, NMI.                                                 */
             App_Fault_ISR,                                              /*  3, Hard Fault.                                          */
             App_Spurious_ISR,                                           /*  4, Memory Management.                                   */
             App_Spurious_ISR,                                           /*  5, Bus Fault.                                           */
             App_Spurious_ISR,                                           /*  6, Usage Fault.                                         */
             App_Spurious_ISR,                                           /*  7, Reserved.                                            */
             App_Spurious_ISR,                                           /*  8, Reserved.                                            */
             App_Spurious_ISR,                                           /*  9, Reserved.                                            */
             App_Spurious_ISR,                                           /* 10, Reserved.                                            */
             App_Spurious_ISR,                                           /* 11, SVCall.                                              */
             App_Spurious_ISR,                                           /* 12, Debug Monitor.                                       */
             App_Spurious_ISR,                                           /* 13, Reserved.                                            */
             OS_CPU_PendSVHandler,                                       /* 14, PendSV Handler.                                      */
             OS_CPU_SysTickHandler,                                      /* 15, uC/OS-III Tick ISR Handler.                          */


             BSP_IntHandlerWWDG,                                         /* 16, INTISR[  0]  Window Watchdog.                        */
             BSP_IntHandlerPVD,                                          /* 17, INTISR[  1]  PVD through EXTI Line Detection.        */
             BSP_IntHandlerRTC,                                          /* 18, INTISR[  2]  Tamper and TimeStamp Interrupt.         */
             BSP_IntHandlerFLASH,                                        /* 19, INTISR[  3]  RTC Wakeup Global Interrupt.            */
             BSP_IntHandlerRCC,                                          /* 20, INTISR[  4]  FLASH Global Interrupt.                 */
             BSP_IntHandlerEXTI0_1,                                      /* 21, INTISR[  5]  RCC Global Interrupt.                   */
             BSP_IntHandlerEXTI2_3,                                      /* 22, INTISR[  6]  EXTI Line0 Interrupt.                   */
             BSP_IntHandlerEXTI4_15,                                     /* 23, INTISR[  7]  EXTI Line1 Interrupt.                   */
             BSP_IntHandlerTSC,                                          /* 24, INTISR[  8]  EXTI Line2 Interrupt.                   */
             BSP_IntHandlerDMA_CH1,                                      /* 25, INTISR[  9]  EXTI Line3 Interrupt.                   */
             BSP_IntHandlerDMA_CH2_3,                                    /* 26, INTISR[ 10]  EXTI Line4 Interrupt.                   */
             BSP_IntHandlerDMA_CH4_5,                                    /* 27, INTISR[ 11]  DMA Channel0 Global Interrupt.          */
             BSP_IntHandlerADC_COMP,                                     /* 28, INTISR[ 12]  DMA Channel1 Global Interrupt.          */
             BSP_IntHandlerTIM1_BRK_UP_TRG_COM,                          /* 29, INTISR[ 13]  DMA Channel2 Global Interrupt.          */
             BSP_IntHandlerTIM1_CC,                                      /* 30, INTISR[ 14]  DMA Channel3 Global Interrupt.          */
             BSP_IntHandlerTIM2,                                         /* 31, INTISR[ 15]  DMA Channel4 Global Interrupt.          */
             BSP_IntHandlerTIM3,                                         /* 32, INTISR[ 16]  DMA Channel5 Global Interrupt.          */
             BSP_IntHandlerTIM6_DAC,                                     /* 33, INTISR[ 17]  DMA Channel6 Global Interrupt.          */
             BSP_IntHandlerTIM7,                     					/* 34, RESERVED INTERRUPT                                   */
             BSP_IntHandlerTIM14,                                        /* 35, INTISR[ 19]  ADC1, ADC2 & ADC3 Global Interrupt.     */
             BSP_IntHandlerTIM15,                                        /* 36, INTISR[ 20]  CAN1 TX  Interrupts.                    */
             BSP_IntHandlerTIM16,                                        /* 37, INTISR[ 21]  CAN1 RX0 Interrupts.                    */
             BSP_IntHandlerTIM17,                                        /* 38, INTISR[ 22]  CAN1 RX1 Interrupt.                     */
             BSP_IntHandlerI2C1,                                         /* 39, INTISR[ 23]  CAN1 SCE Interrupt.                     */
             BSP_IntHandlerI2C2,                                         /* 40, INTISR[ 24]  EXTI Line[9:5] Interrupt.               */
             BSP_IntHandlerSPI1,                                         /* 41, INTISR[ 25]  TIM1 Break  Interrupt & TIM9 Glb int.   */
             BSP_IntHandlerSPI2,                                         /* 42, INTISR[ 26]  TIM1 Update Interrupt & TIM10 Glb int.  */
             UART1_IRQService,/*BSP_IntHandlerUSART1,*/	            	/* 43, INTISR[ 27]  TIM1 Trig & Commutation Interrupts...   */
             BSP_IntHandlerUSART2,                                       /* 44, INTISR[ 28]  TIM1 Capture Compare Interrupt.         */
             App_Reserved_ISR,                                           /* 45, RESERVED INTERRUPT                                   */
             BSP_IntHandlerCEC,                                          /* 46, INTISR[ 30]  TIM2 Global Interrupt.                  */
             App_Reserved_ISR,                                           /* 47, RESERVED INTERRUPT                                   */
         };


/*
*********************************************************************************************************
*                                            App_NMI_ISR()
*
* Description : Handle Non-Maskable Interrupt (NMI).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) Since the NMI is not being used, this serves merely as a catch for a spurious
*                   exception.
*********************************************************************************************************
*/

static  void  App_NMI_ISR(void)
{
    while (DEF_TRUE)
    {
        ;
    }
}


/*
*********************************************************************************************************
*                                             App_Fault_ISR()
*
* Description : Handle hard fault.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Fault_ISR(void)
{
    while (DEF_TRUE)
    {
        ;
    }
}


/*
*********************************************************************************************************
*                                           App_Spurious_ISR()
*
* Description : Handle spurious interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Spurious_ISR(void)
{
    while (DEF_TRUE)
    {
        ;
    }
}


/*
*********************************************************************************************************
*                                           App_Reserved_ISR()
*
* Description : Handle spurious interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_Reserved_ISR(void)
{
    while (DEF_TRUE)
    {
        ;
    }
}
