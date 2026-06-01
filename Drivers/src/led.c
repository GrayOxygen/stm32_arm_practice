#include "led.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
// 练习定时器

void LED_Init(void)
{
    // 1. 使能 GPIOB 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 2. 配置 GPIOB 的第0、1、2、3引脚为推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出：因为数据来源是CPU的寄存器，不是亲手写的，而是来着TIM外设，所以是复用模式
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 3. 默认关闭所有LED（假设LED是低电平点亮）
    GPIO_SetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6);
}

// 初始化 TIM3 的 PWM 输出 (PC6, PC7, PC8 对应 TIM3 的 CH1, CH2, CH3)；TIM 2-5 通用定时器
void LED_PWM_Init(void)
{
    // 不用单独设置GPIO，LED_Init里初始化了
    // GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 开启 GPIOC 和 TIM3 的时钟
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); // 完全重映射：GPIOC的6,7,8分别映射到TIM3的CH1, CH2, CH3

    // // 配置 PC6, PC7, PC8 为复用推挽输出
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 配置 TIM3 基础参数
    TIM_TimeBaseStructure.TIM_Period = 255;                     // PWM 周期
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;               // 预分频，72MHz / 72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 不分频,Clock Division一般用于滤波器（有抖动电压值时），PWM不需要，所以设为0
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 一般向上计数即可
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             // 初始化时基单元

    // 配置 TIM3 的 3 个 PWM 通道
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // 不管上行计数还是下行计数，PWM1模式都是当CNT < CCRx时，输出为有效电平，否则为无效电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // 这里设置低电平点亮（下行），如果你的LED是高电平点亮，后续设置占空比时需要用255减去实际值

    // 初始化通道 1 (PC6 - Red)
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    // 初始化通道 2 (PC7 - Green)
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    // 初始化通道 3 (PC8 - Blue)
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

    // 5. 启动 TIM3
    TIM_Cmd(TIM3, ENABLE);
    TIM_CtrlPWMOutputs(TIM3, ENABLE); // 启动 PWM 输出
}

// 输出比较
// 设置 RGB 颜色 (r, g, b: 0~255)
void LED_Set_Color(uint8_t r, uint8_t g, uint8_t b)
{
    // 假设低电平点亮（共阳极），用 255 去减。如果是高电平点亮，直接写 r, g, b 即可。
    TIM_SetCompare1(TIM3, 255 - r); // 修改通道1的占空比
    TIM_SetCompare2(TIM3, 255 - g); // 修改通道2的占空比
    TIM_SetCompare3(TIM3, 255 - b); // 修改通道3的占空比
}
