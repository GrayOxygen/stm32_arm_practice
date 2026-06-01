#ifndef __MY_TIMER_H__
#define __MY_TIMER_H__
#include "stdint.h"
// 声明外部全局变量（告诉其他文件，这两个变量在别处定义了）
extern uint16_t catch_tim5_reg;
extern uint8_t catch_status;

void tim5_ch1_init(uint32_t Period, uint32_t Prescaler);

#endif