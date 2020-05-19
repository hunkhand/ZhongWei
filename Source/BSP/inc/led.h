#ifndef _LED_H_
#define _LED_H_

#include "stm32f0xx.h"
#include  <includes.h>

void LED_Init(void);
void LED_Open(void);
void LED_Close(void);

extern OS_TMR TMR_LED_Short_Light;
extern OS_TMR TMR_LED_Double_Light;

extern unsigned long ulLedStatue;
void LED_Double_Light(void *p_tmr, void *p_arg);
void LED_Short_Light(void *p_tmr, void *p_arg);


#endif


