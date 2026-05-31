// 防止头文件重复注入
#ifndef __BEEP_H__
#define __BEEP_H__
#include "stm32f10x.h"
#include "bitband.h"

// ODR寄存器修改方式
#define USE_ODR_REG 0

// ODR寄存器中的第九位置0或1; 写了位段，就不用下面积设置ODR, BRR, BSRR，可以直接设置了，相当于给PC 9引脚取了别名BEEP,直接设置BEEP就可以了
#define BEEP PCout(9)
#define BEEP_ON() BEEP = 0
#define BEEP_OFF() BEEP = 1
#define BEEP_TOGGLE() BEEP = !BEEP

// #if USE_BSRR_REG
//  查看设计图，是一个PNP管，低电平导通，会响，高电平截止
// #define BEEP_ON()		do{ GPIOC->ODR &=~(1<<9); }while(0)
// #define BEEP_OFF()	do{ GPIOC->ODR |=(1<<9); }while(0)
// #else
////BSRR操作是原子性的，不会被打断，避免了修改ODR寄存器的风险
// #define BEEP_ON()		do{ GPIOC->BRR |=(1<<9); }while(0)
// #define BEEP_OFF()  do{ GPIOC->BSRR |=(1<<9); }while(0)
// #endif

void beep_init(void);
void beep_init_std(void);
void remap_init(void);

#endif
