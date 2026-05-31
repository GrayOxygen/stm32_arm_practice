#ifndef __LED_H__
#define __LED_H__

#include "bitband.h"

#define LED_R PCout(8) // 定义LED_R为GPIOB的第8号引脚的位段别名
#define LED_G PCout(7) // 定义LED_G为GPIOB的第7号引脚的位段别名
#define LED_B PCout(6) // 定义LED_B为GPIOB的第6号引脚的位段别名

#define LED_R_On() (LED_R = 0)
#define LED_R_Off() (LED_R = 1)
#define LED_R_Toggle() (LED_R = !LED_R)

#define LED_G_On() (LED_G = 0)
#define LED_G_Off() (LED_G = 1)
#define LED_G_Toggle() (LED_G = !LED_G)

#define LED_B_On() (LED_B = 0)
#define LED_B_Off() (LED_B = 1)
#define LED_B_Toggle() (LED_B = !LED_B)

void LED_Init(void);

// 假设你的 RGB 灯接在 TIM3 的 CH1(PC6), CH2(PC7), CH3(PC8)
// 这里的 PWM 周期最大值（ARR）设为 255，刚好对应 0-255 的亮度
#define PWM_MAX_VALUE 255

// 初始化 RGB 引脚的 PWM 功能
void LED_PWM_Init(void);

// 设置 RGB 颜色 (r, g, b 的取值范围均为 0~255)
void LED_Set_Color(uint8_t r, uint8_t g, uint8_t b);

#endif
