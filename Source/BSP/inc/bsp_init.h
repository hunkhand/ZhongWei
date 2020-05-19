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
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : FF
*                 MD
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP present pre-processor macro definition.
*
*           (2) This file and its dependencies requires IAR v6.20 or later to be compiled.
*
*********************************************************************************************************
*/

#ifndef  BSP_PRESENT
#define  BSP_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_MODULE
#define  BSP_EXT
#else
#define  BSP_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdio.h>
#include  <stdarg.h>

#include  <cpu.h>
#include  <cpu_core.h>

#include  <lib_def.h>
#include  <lib_ascii.h>

#include  <stm32f0xx_gpio.h>

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             PERIPH DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                 MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               INT DEFINES
*********************************************************************************************************
*/

#define  BSP_INT_ID_WWDG                                   0    /* Window WatchDog Interrupt                            */
#define  BSP_INT_ID_PVD                                    1    /* PVD through EXTI Line detection Interrupt            */
#define  BSP_INT_ID_RTC                                    2    /* RTC global wakeup                                    */
#define  BSP_INT_ID_FLASH                                  3    /* FLASH global Interrupt                               */
#define  BSP_INT_ID_RCC                                    4    /* RCC global Interrupt                                 */
#define  BSP_INT_ID_EXTI0_1                                5    /* EXTI Line[1:0] Interrupt                             */
#define  BSP_INT_ID_EXTI2_3                                6    /* EXTI Line[3:2] Interrupt                             */
#define  BSP_INT_ID_EXTI4_15                               7    /* EXTI Line15 and EXTI4 Interrupts                     */
#define  BSP_INT_ID_TSC                                    8    /* TSC Touch Sensing Interrupt                          */
#define  BSP_INT_ID_DMA_CH1                                9    /* DMA Channel 1 Interrupt                              */
#define  BSP_INT_ID_DMA_CH2_3                             10    /* DMA channel 2 and 3 interrupts                       */
#define  BSP_INT_ID_DMA_CH4_5                             11    /* DMA Channel 4 AND 5 Interrupt                        */
#define  BSP_INT_ID_ADC_COMP                              12    /* ADC and comparator 1 and 2 Interrupt                 */
#define  BSP_INT_ID_TIM1_BRK_UP_TRG_COM                   13    /* TIM1 Break, update,trigger and commutation interrupt */
#define  BSP_INT_ID_TIM1_CC                               14    /* TIM1_CC Capture Compare Interrupt                    */
#define  BSP_INT_ID_TIM2                                  15    /* TIM2 global Interrupt                                */
#define  BSP_INT_ID_TIM3                                  16    /* TIM3 global Interrupt                                */
#define  BSP_INT_ID_TIM6_DAC                              17    /* TIM6 global Interrupt and DAC underrun interrupt     */
#define  BSP_INT_ID_TIM7                                  18    /* TIM7 global Interrupt and DAC underrun interrupt     */
#define  BSP_INT_ID_TIM14                                 19    /* TIM14 global interrupt                               */
#define  BSP_INT_ID_TIM15                                 20    /* TIM15 global interrupt                               */
#define  BSP_INT_ID_TIM16                                 21    /* TIM16 global interrupt                               */
#define  BSP_INT_ID_TIM17                                 22    /* TIM17 global interrupt                               */
#define  BSP_INT_ID_I2C1                                  23    /* I2C1 global interrupt                                */
#define  BSP_INT_ID_I2C2                                  24    /* I2C2 global interrupt                                */
#define  BSP_INT_ID_SPI1                                  25    /* SPI1 globbal interrupt                               */
#define  BSP_INT_ID_SPI2                                  26    /* SPI2 global interrupt                                */
#define  BSP_INT_ID_USART1                                27    /* USART1 global Interrupt                              */
#define  BSP_INT_ID_USART2                                28    /* USART2 global Interrupt                              */
#define  BSP_INT_ID_RSVD_01                               29    /* RESERVED                                             */
#define  BSP_INT_ID_CEC                                   30    /* CEC global interrupt                                 */
#define  BSP_INT_ID_RSVD_02                               31    /* RESERVED                                             */


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        BSP_Init                          (void);

void        BSP_IntDisAll                     (void);

CPU_INT32U  BSP_CPU_ClkFreq                   (void);

/*
*********************************************************************************************************
*                                           INTERRUPT SERVICES
*********************************************************************************************************
*/

void        BSP_IntInit                       (void);

void        BSP_IntEn                         (CPU_DATA       int_id);

void        BSP_IntDis                        (CPU_DATA       int_id);

void        BSP_IntClr                        (CPU_DATA       int_id);

void        BSP_IntVectSet                    (CPU_DATA       int_id,
                                               CPU_FNCT_VOID  isr);

void        BSP_IntPrioSet                    (CPU_DATA       int_id,
                                               CPU_INT08U     prio);

void         BSP_IntHandlerWWDG                 (void);
void         BSP_IntHandlerPVD                  (void);
void         BSP_IntHandlerRTC                  (void);
void         BSP_IntHandlerFLASH                (void);
void         BSP_IntHandlerRCC                  (void);
void         BSP_IntHandlerEXTI0_1              (void);
void         BSP_IntHandlerEXTI2_3              (void);
void         BSP_IntHandlerEXTI4_15             (void);
void         BSP_IntHandlerTSC                  (void);
void         BSP_IntHandlerDMA_CH1              (void);
void         BSP_IntHandlerDMA_CH2_3            (void);
void         BSP_IntHandlerDMA_CH4_5            (void);
void         BSP_IntHandlerADC_COMP             (void);
void         BSP_IntHandlerTIM1_BRK_UP_TRG_COM  (void);
void         BSP_IntHandlerTIM1_CC              (void);
void         BSP_IntHandlerTIM2                 (void);
void         BSP_IntHandlerTIM3                 (void);
void         BSP_IntHandlerTIM6_DAC             (void);
void         BSP_IntHandlerTIM7                 (void);
void         BSP_IntHandlerTIM14                (void);
void         BSP_IntHandlerTIM15                (void);
void         BSP_IntHandlerTIM16                (void);
void         BSP_IntHandlerTIM17                (void);
void         BSP_IntHandlerI2C1                 (void);
void         BSP_IntHandlerI2C2                 (void);
void         BSP_IntHandlerSPI1                 (void);
void         BSP_IntHandlerSPI2                 (void);
void         BSP_IntHandlerUSART1               (void);
void         BSP_IntHandlerUSART2               (void);
void         BSP_IntHandlerCEC                  (void);


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/


#endif                                                          /* End of module include.                               */

