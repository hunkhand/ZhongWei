
#include "led.h"

OS_TMR TMR_LED_Short_Light;
OS_TMR TMR_LED_Double_Light;

unsigned long ulLedStatue = 0;


void LED_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;  

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LED_Close();
}

void LED_Open(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void LED_Close(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
}

void LED_Short_Light(void *p_tmr, void *p_arg)
{
    LED_Close();
}

void LED_Double_Light(void *p_tmr, void *p_arg)
{
    OS_ERR  Err;
    u32 OpenAgain = *(u32 *)p_arg;

    if(OpenAgain >= 2)
    {
        *(u32 *)p_arg = 1;
        LED_Close();
        OSTmrStart(&TMR_LED_Double_Light, &Err);
    }
    else if(1 == OpenAgain)
    {
        *(u32 *)p_arg = 0;
        LED_Open();
        OSTmrStart(&TMR_LED_Double_Light, &Err);
    }
    else
    {
        LED_Close();
    }
}


