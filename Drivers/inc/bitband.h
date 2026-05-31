#ifndef __BITBAND_H__
#define __BITBAND_H__

#include "stm32f10x.h"
/*
手册里给的公式
bit_word_addr = bit_band_base + (byte_offset x 32) + (bit_number × 4)
其中：
bit_word_addr是位段别名区的基地址，它映射到某个目标位。
bit_band_base是位段别名区的起始地址。
byte_offset是包含目标位的字节在位段里的序号
bit_number是目标位所在位置(0-31)
*/
// (1+2)*5
// 1+2*5
#define BIT_WORD_ADDR(bit_band_base, byte_offset, bit_number) \
	((uint32_t)(bit_band_base) + ((uint32_t)(byte_offset) * 32) + ((uint32_t)(bit_number) * 4))
/*
	如何根据位段区地址得到别名区的起始地址
	GPIOC->ODR=0x40000000+0x1000+0x10000+0x0C=0x4001100C
	获取0x40000000+0x2000000(32MB)
	(0x4001100C & 0xF0000000) +0x2000000  = 位段别名区的起始地址
	如何获取位段别名区的字节偏移地址
	GPIOC->ODR地址为0x4001100C
	（0x4001100C & 0x000FFFFF）=0x0001100C
	addr:寄存器的地址
*/
#define BIT_TO_ALIAS(addr, bit_number)                 \
	BIT_WORD_ADDR(                                     \
		(((uint32_t)(addr)) & 0xF0000000) + 0x2000000, \
		((uint32_t)(addr)) & 0x000FFFFF,               \
		bit_number)
// 获取端口的IDR寄存器的位段别名区的地址
#define PPAin(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOA->IDR, bit_number))
#define PPBin(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOB->IDR, bit_number))
#define PPCin(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOC->IDR, bit_number))
#define PPDin(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOD->IDR, bit_number))
// 端口的ODR寄存器的位段别名区地址
#define PPAout(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOA->ODR, bit_number))
#define PPBout(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOB->ODR, bit_number))
#define PPCout(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOC->ODR, bit_number))
#define PPDout(bit_number) ((volatile uint32_t *)BIT_TO_ALIAS(&GPIOD->ODR, bit_number))

// 位在位段别名区的地址值的值PAin(5)  IDR中PA5对应地址值中的数据
#define PAin(bit_number) (*PPAin(bit_number))
#define PBin(bit_number) (*PPBin(bit_number))
#define PCin(bit_number) (*PPCin(bit_number))
#define PDin(bit_number) (*PPDin(bit_number))
// 位在位段别名区的地址值中的值PAout(5)  ODR中PA5对应地址值中的数据
#define PAout(bit_number) (*PPAout(bit_number))
#define PBout(bit_number) (*PPBout(bit_number))
#define PCout(bit_number) (*PPCout(bit_number))
#define PDout(bit_number) (*PPDout(bit_number))

#endif
