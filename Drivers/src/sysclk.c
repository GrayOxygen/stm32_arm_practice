#include "sysclk.h"

void sysclk_init(void)
{
	// 1，打开外部时钟
	RCC_HSEConfig(RCC_HSE_ON);
	// 2，等待外部高速晶振且就绪
	while (RCC_WaitForHSEStartUp() == ERROR)
	{
	};
	// 3，配置锁相环的时钟源和倍频系数
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	// 4，使能锁相环
	RCC_PLLCmd(ENABLE);
	// 5， 等待锁相环工作稳定
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	};
	// 6，配置AHB的分频器
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	// 7，配置APB1的分频器
	RCC_PCLK1Config(RCC_HCLK_Div2);
	// 8，配置APB2的分频器
	RCC_PCLK2Config(RCC_HCLK_Div1);
	// 9，配置系统时钟来源
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	// 10，等待配置成功
	while (RCC_GetSYSCLKSource() != 0x08)
	{
	};
}
