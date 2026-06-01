#include "my_usart.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"

/**
 * PA9 默认复用功能USART1_TX
 * PA10 默认复用功能USART1_RX
 * USART1 的时钟来自 APB2，最高72MHz
 */
void usart1_init(void)
{
    GPIO_InitTypeDef GPIO9_InitStructure;
    GPIO_InitTypeDef GPIO10_InitStructure;
    
    USART_InitTypeDef usart1_instruct;
    // 1. 使能 USART1 和 GPIOA 时钟 TODO 没用重映射，不用开启AFIO时钟了
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
    GPIO9_InitStructure.GPIO_Pin = GPIO_Pin_9;       // USART1_TX
    GPIO9_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽
    GPIO9_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO9_InitStructure);
    // 初始化PA10为输入，接收数据
    GPIO10_InitStructure.GPIO_Pin = GPIO_Pin_10;    // USART1_RX
    GPIO10_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(GPIOA, &GPIO10_InitStructure);
    // 配置串口参数
    usart1_instruct.USART_BaudRate = 115200;                                    // 波特率
    usart1_instruct.USART_WordLength = USART_WordLength_8b;                     // 8位数据位
    usart1_instruct.USART_StopBits = USART_StopBits_1;                          // 1个停止位
    usart1_instruct.USART_Parity = USART_Parity_No;                             // 无奇偶校验
    usart1_instruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    usart1_instruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 同时使能发送和接收
    USART_Init(USART1, &usart1_instruct);
    // 启动串口
    USART_Cmd(USART1, ENABLE);
}

void usart1_putchar(int data)
{
    // 等待发送缓冲区空
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;
    // 发送数据
    USART_SendData(USART1, (uint8_t)data);
}

uint16_t usart1_getchar(void)
{
    // 等待接收缓冲区非空
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        ;
    // 读取数据
    return (uint16_t)USART_ReceiveData(USART1);
}