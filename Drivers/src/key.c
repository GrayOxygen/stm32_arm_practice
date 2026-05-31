#include "key.h"
#include "Beep.h"
#include "led.h"
#include "systick.h"

// 写中断：配置nvic，exti，中断函数，以及main中仅调用一次的中断分组
// 初始化按键函数
void key_init(void)
{
	// PA0
	GPIO_InitTypeDef gpio_instruct;
	NVIC_InitTypeDef nvic_instruct;
	EXTI_InitTypeDef exti_instruct;
	// 开启时钟，APB2（所有外设都在这个上面）  GPIOA是按键的引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	// 设置GPIO的模式
	gpio_instruct.GPIO_Pin = GPIO_Pin_0;
	gpio_instruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_instruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入，防止外部上拉电阻失效，多一个上拉的作用
	GPIO_Init(GPIOA, &gpio_instruct);

	// 中断配置==>NVIC配置+EXTI配置
	// 初始化NVIC, GPIO没有中断，由外部中断处理
	nvic_instruct.NVIC_IRQChannel = EXTI0_IRQn;			 // 选择哪一个信号触发中断
	nvic_instruct.NVIC_IRQChannelCmd = ENABLE;			 // 开启对应通道
	nvic_instruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级，先设置一个值2
	nvic_instruct.NVIC_IRQChannelSubPriority = 2;		 // 响应优先级，先设置一个值2
	NVIC_Init(&nvic_instruct);

	// 初始化EXTI
	exti_instruct.EXTI_Line = EXTI_Line0;			   // 初始化哪一个外部中断线
	exti_instruct.EXTI_LineCmd = ENABLE;			   // 开启外部中断线
	exti_instruct.EXTI_Mode = EXTI_Mode_Interrupt;	   // 外部中断线的模式(事件是用于触发事件，不是中断，这里我们需要中断，所以选择中断模式)
	exti_instruct.EXTI_Trigger = EXTI_Trigger_Falling; // 触发边沿:下降沿触发，按键是低电平有效的，所以按下时会产生下降沿
	EXTI_Init(&exti_instruct);

	// EXTI配置: GPIOA的第0号引脚对应EXTI的第0号线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
}

// 不准确的延时
// void delay_ms(unsigned int count)
// {
// 	while (count--)
// 		;
// }

// 支持长按和多次触发
#define key_test 0
#define key_long_press 1

#if key_test
void key_scan(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
	{
		// 延时
		delay_ms(500);
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
		{
			// 如果按下则切换蜂鸣器状态
			// 存在阻塞的问题
			while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
			{
			};
			BEEP = !BEEP;
		}
	}
}
#elif key_long_press
uint32_t key_flag = 0;
uint32_t press_start_time = 0; // 记录按键按下的起始时间

// 节省空间定义：支持长按和多次触发，key_flag[31] 按下标志，key_flag[30] 松开标志，key_flag[0-29] 按键按下的次数
void key_scan(void)
{
	// 判断按键是否按下
	if ((key_flag & (1U << 31)))
	{ // 按键按下
		uint32_t time = 0;
		// 等待按键抬起
		while (!(key_flag & (1 << 30)))
		{
		};
		// 根据flag中0~29的数值，判断按键按下的时长
		time = (key_flag & (0x3FFFFFFF)) * key_delay_ms;
		// 清除标志位
		key_flag = 0;
		// 判断长短按
		if (time > KEY_LONG_PRESSED)
		{ // 长按静音
			BEEP = 1;
		}
		else
		{ // 短按发声
			BEEP = 0;
		}
	}
}

#else
void key_scan(uint8_t support_long_press)
{
	// 0按下未释放，1 按键已松开，可重新检测按键状态
	static uint8_t key_up = 1;

	// 单次触发：响-静音
	if (support_long_press == 0)
	{
		// 读取电平状态，按下了，且为低电平
		if (key_up && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
		{
			delay_ms(500); // 消抖
			if (key_up && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
			{
				key_up = 0;
				BEEP_TOGGLE(); // 切换蜂鸣器状态
			}
		}
		else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) != Bit_RESET)
		{ // 没按下
			key_up = 1;
		}
	}
	else
	{ // 长按多次触发：响-静音-响-静音...
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
		{
			// 消抖
			delay_ms(500);
			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
			{
				BEEP_TOGGLE(); // 切换蜂鸣器状态
			}
		}
	}
}
#endif
