#ifndef __MY_USART_H__
#define __MY_USART_H__

#include <stdint.h>

void usart1_init(void);
void usart1_putchar(int data);
uint16_t usart1_getchar(void);
#endif
