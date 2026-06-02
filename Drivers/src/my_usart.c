#include "my_usart.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include <stdio.h> // 必须包含这个头文件，因为用到了 FILE

// 用于实现一句一句的发送和读取
// 定义最大接收长度
#define UART_REC_LEN 100

// 接收缓冲区，用来存放攒起来的一句话
uint8_t UART_RX_BUF[UART_REC_LEN];

// 接收状态标记变量
// bit15 (0x8000): 接收完成标志（收到回车符后由中断置1）
// bit14 (0x4000): 接收到回车符 '\r' 的标志
// bit13~0: 当前接收到的有效字符个数
uint16_t UART_RX_STA = 0;

extern int usart1_get_index = 0; // 头部
extern int usart1_rev_index = 0; // 尾部
extern int usart1_idle_flag = 0; // 空闲标志，1位串口空闲，0表示串口繁忙

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

    // 配置中断优先级，用来缓冲接到的数据

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能接收, 空闲中断
    USART_ITConfig(USART1, USART_IT_RXNE | USART_IT_IDLE, ENABLE);

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

char usart1_getchar_with_interrupt_buf(void)
{
    char data;
    // 中断读取的数据塞入到的buffer里读取
    while (usart1_get_index >= usart1_rev_index)
    {
        data = usart1_recv_buf[usart1_get_index];
        if (usart1_get_index >= usart1_rev_index)
        {
            usart1_get_index = usart1_rev_index = usart1_idle_flag = 0;
        }
    }
    return data;
}

// 必须使用microLib微库（KEIL在TARGET可以勾选）; 类似重写
int fputc(int data, FILE *file)
{
    return usart1_putchar(data);
}

int fgetc(FILE *file)
{
    return usart1_getchar();
}

// 通过操作寄存器，操作串口
void usart1_init_by_manipulate_register(void)
{
    // 开启时钟： AFIO GPIOA USART1
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH &= ~(0xF << (4 * (9 - 8))) | (0xF << (10 - 8)); // 先清零
    GPIOA->CRH |= (0xB<<(4*(9-8))) | (0x4<<(10-8))); // PA9 复用推挽输出，PA10 上拉输入
    // 配置串口参数：波特率115200，8N1，无流控
    uint32_t pclk2 = 72000000; // APB2时钟频率
    // 其实就是+0.5来实现四舍五入，避免误差过大导致波特率不准
    uint32_t usartdiv = (pclk2 + (115200 / 2)) / 115200;
    USART1->BRR = (uint16_t)(usartdiv & 0XFFF0) | ((usartdiv & 0xF) >> 1); // 设置波特率寄存器
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;                             // 使能发送和接收
    USART1->CR2 = 0x0000;                                                  // 1个停止位
    USART1->CR3 = 0x0000;                                                  // 无硬件流控
    USART1->CR1 |= (1 << 13);                                              // 使能接收中断
}
// USARTDIV = Fck*1/16/Baud

uint8_t usart1_putchar_by_manipulate_register(int data)
{
    // 等待发送缓冲区空
    while ((USART1->SR & USART_SR_TXE) == 0)
        ;
    // 发送数据，只取8位
    USART1->DR = (data & 0xFF);
    return data;
}

uint8_t usart1_getchar_by_manipulate_register(void)
{
    // 等待接收缓冲区非空
    while ((USART1->SR & USART_SR_RXNE) == 0)
        ;
    // 读取数据：只取8位
    return (uint8_t)(USART1->DR & 0xFF);
}
