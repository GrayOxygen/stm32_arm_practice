#ifndef __SYSTICK_H__
#define __SYSTICK_H__
#include "stm32f10x.h"
void systick_init(uint32_t SysTick_CLKSource);
void delay_ms(uint32_t ms);
void systick_stop(void);

void systick_delay_ms(uint32_t ms); 
uint8_t systick_start(uint32_t ms);
#endif
