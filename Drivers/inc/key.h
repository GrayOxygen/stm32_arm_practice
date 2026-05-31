#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f10x.h"
#include "bitband.h"

#define KEY PAin(0)

#define key_delay_ms 30      // 用于消抖的间隔时间，毫米
#define KEY_LONG_PRESSED 300 // 长按时间阈值，毫秒

extern uint32_t key_flag;

void key_init(void);
// void key_scan(uint8_t support_long_press);

#endif
