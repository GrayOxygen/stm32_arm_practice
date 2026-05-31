#include "systick.h"

// 默认值
uint32_t fac_ms = 9000;
uint32_t fac_us = 9;
void systick_init(uint32_t SysTick_CLKSource)
{
    SysTick->LOAD = SysTick->VAL = 0; // 清空计数器
    if (SysTick->CTRL == SysTick_CLKSource_HCLK)
    {
        // 选择AHB作为时钟源，1即设置为内核时钟，用的是72MHz的时钟，不分频
        SysTick->CTRL |= 1 << 2;
        fac_ms = 72000;
        fac_us = 72;
    }
}

// 精确延时，毫秒; 公式：T = VAL * PSC / f (练习下，熟悉原理)
void systick_delay_ms(uint32_t ms)
{
    // 计算溢出多少次 ， 溢出的数字一共24位，所以是0x00FFFFFF
    // ms*fac_ms（一共要记多少个数） / 0x00FFFFFF（每次溢出记多少个数）;
    int count = ms * fac_ms / 0x00FFFFFF;
    // 不能保证每次都整除，所以要计算出余数
    uint32_t remain = ms * fac_ms % 0x00FFFFFF;

    // 原理：load的值会加载到VAL寄存器，VAL寄存器会递减计数，当计数到0时会产生一次溢出，设置了中断的话会进入中断服务函数
    SysTick->VAL = 0;
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL &= ~((1 << 16) | 0x01); // 先关闭定时器，禁用中断，选择外部时钟源(/8)，清除标志位
    if (count)
    {
        // 重新开启
        SysTick->CTRL |= 0x01;
        while (count--)
        {
            // 等待标志位 16位如果是1 则表示溢出1次；数完时，为1，不为1则等待
            while (!(SysTick->CTRL & (1 << 16)))
            {
            };
            // 清除标志位
            SysTick->CTRL &= ~(1 << 16);
        }
        // 关闭定时器
        SysTick->CTRL &= ~(0x01);
    }
    if (remain)
    { // 再次计数：针对剩余数
        // 剩余值计数
        SysTick->LOAD = remain;
        // 启动定时器
        SysTick->CTRL |= 0x01;
        // 等待溢出
        while (!(SysTick->CTRL & (1 << 16)))
        {
        };
        // 关闭定时器  并且清除标志位
        SysTick->CTRL &= ~((1 << 16) | 0x01);
    }
    SysTick->VAL = SysTick->LOAD = 0;
}

// 对延时函数systick_delay_ms进行优化  优化掉systick初始化函数（使用时，调这个）
void delay_ms(uint32_t ms)
{
    // 1.停止定时器，禁用中断，选择AHB为时钟源，清除标志位
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE; // 0x04
    // 清0VAL寄存器
    SysTick->VAL = 0;
    SysTick->LOAD = 72000 - 1;
    // 启动定时器
    SysTick->CTRL |= SysTick_CTRL_ENABLE; // SysTick_CTRL_ENABLE  0x01
    while (ms--)
    {
        // 等待标志位
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG))
        {
        };
        // 清除标志位
        SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG;
    }
    // 关闭定时器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}

// 开启systick中断
uint8_t systick_start(uint32_t ms)
{
    // 关闭定时器，禁用中断，选择外部时钟源(/8)，设置重装载器
    SysTick->CTRL = 0;
    if (ms >= 1300)
    {
        return 0;
    }
    SysTick->LOAD = ms * fac_ms - 1;
    // 清空VAL寄存器
    SysTick->VAL = 0;
    // 开启中断，开启定时器
    SysTick->CTRL = SysTick_CTRL_TICKINT | SysTick_CTRL_ENABLE;
}

// 关闭中断
void systick_stop(void)
{
    SysTick->CTRL = 0;
}
