#ifndef __MY_USART_H__
#define __MY_USART_H__

#include <stdint.h>

void usart1_init(void);
void usart1_putchar(int data);
uint16_t usart1_getchar(void);

// 1. 定义一个全局的环形缓冲区（相当于一个固定大小的队列）

// 定义最大接收长度
#define UART_REC_LEN 100

// 接收缓冲区，用来存放攒起来的一句话
extern uint8_t UART_RX_BUF[UART_REC_LEN];

extern int usart1_get_index = 0; // 头部
extern int usart1_rev_index = 0; // 尾部
extern int usart1_idle_flag = 0; // 空闲标志，1位串口空闲，0表示串口繁忙

// 接收状态标记变量
// bit15 (0x8000): 接收完成标志（收到回车符后由中断置1）
// bit14 (0x4000): 接收到回车符 '\r' 的标志
// bit13~0: 当前接收到的有效字符个数
extern uint16_t UART_RX_STA;
#endif
