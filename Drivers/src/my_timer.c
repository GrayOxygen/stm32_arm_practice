#include "my_timer.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"

// 最后一次定时器当中的计数值
uint16_t catch_tim5_reg = 0x00;
// 8位：最高位表示完成捕获，次高位捕获到下降沿，除这两位外的其他位表示捕获到的计数值，即溢出次数
uint8_t catch_status = 0x00;

void tim5_ch1_init(uint32_t Period, uint32_t Prescaler)
{
    // 不用单独设置GPIO，LED_Init里初始化了
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef nvic_instruct;

    // 开启 GPIOC 和 TIM3 的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    //  初始化GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    //  按键是高平到低电平为按下，外接了一个上拉电阻，这里为了确保为高电平，所以上拉
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 TIM5 基础参数
    TIM_TimeBaseStructure.TIM_Period = Period;                  // PWM 周期
    TIM_TimeBaseStructure.TIM_Prescaler = Prescaler - 1;        // 预分频，72MHz / 72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 不分频,Clock Division一般用于滤波器（有抖动电压值时），PWM不需要，所以设为0
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 一般向上计数即可
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);             // 初始化时基单元

    // 捕获通道
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; // 下降沿捕获:高到底，按下按键
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM5, &TIM_ICInitStructure);

    // 通过第一次捕获到时触发的中断，来改变方向，做上升沿捕获:低到高，按键抬起
    // 中断配置==>NVIC配置+EXTI配置
    // 初始化NVIC, GPIO没有中断，由外部中断处理
    nvic_instruct.NVIC_IRQChannel = TIM5_IRQn;           // 选择哪一个信号触发中断
    nvic_instruct.NVIC_IRQChannelCmd = ENABLE;           // 开启对应通道
    nvic_instruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级，先设置一个值2
    nvic_instruct.NVIC_IRQChannelSubPriority = 2;        // 响应优先级，先设置一个值2
    NVIC_Init(&nvic_instruct);

    // tim有自己的中断处理，无需像GPIO那样走EXTI外部总段
    TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE);

    // 5. 启动 TIM5
    TIM_Cmd(TIM5, ENABLE);
}

void TIM5_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)
    { // TIM_ICPolarity_Falling管道里记录的是下降沿
        // 捕获到下降沿，按键按下
        if (catch_status & 0x40) // 已捕获到下降沿，所以，这次捕获是上边沿
        {
            catch_tim5_reg = TIM_GetCounter(TIM5);               // 读取当前计数值，记录按下时的计数值
            catch_status |= 0x80;                                // 设置最高位，表示完成捕获
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Falling); // 等待下一次按键按下
        }
        else
        {
            // 次高位为0：说明是第一次进中断，本次是下降沿（按键按下）
            catch_status = 0;   // 上一次结果清零
            catch_tim5_reg = 0; // 上一次计数器的值
            // 把定时器中的计数值清零
            TIM_SetCounter(TIM5, 0);
            catch_status = 0x40;                                // 次高位置，表示捕获到下降沿，正在等待上升沿
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Rising); // 切换为上升沿捕获，等待按键抬起
        }
    }
    else if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
    { // 定时器跑完才会触发
        if ((catch_status & 0x40))
        { // 捕获到下降沿，正在等待上升沿
            if ((catch_status & 0x3f) == 0x3f)
            { // 计数已经满了，说明按键一直按着没有松开，记录最大值
                catch_tim5_reg = 0xFFFF;
                catch_status |= 0x80;
                TIM_ClearITPendingBit(TIM5, TIM_IT_Update); // 清除中断标志位
                TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);    // 清除中断标志位
                return;
            }
            else
            { // 计数没有满，继续等待，记录溢出次数
                catch_status++;
            }
        }
    }
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update); // 清除中断标志位
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);    // 清除中断标志位
}
