/**
 ******************************************************************************
 * @file    GPIO/IOToggle/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and peripherals
 *          interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "systick.h"
#include "key.h"
#include "my_usart.h"
/** @addtogroup STM32F10x_StdPeriph_Examples
 * @{
 */

/** @addtogroup GPIO_IOToggle
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSV_Handler exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * 用滴答计时器做按键功能
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{ // tick计时器，用来定时检查/更新按键状态
  if (!KEY)
  {
    key_flag |= (1U << 31); // 按键按下，设置高位标志
  }
  else
  {
    key_flag |= (1 << 30); // 按键松开，设置次高位标志
    systick_stop();
  }
  key_flag++;
}

// 每收到数据，中断都会将数据放入ring_buffer，tail指针向后移动
void USART1_IRQHandler(void)
{
  // 判断谁触发的中断
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    // 读取数据，放入环形缓冲区
    uint8_t received_char = USART_ReceiveData(USART1);
    UART_RX_BUF[usart1_rev_index] = received_char;
    usart1_rev_index++;
    if (usart1_rev_index >= UART_REC_LEN)
    {
      usart1_rev_index = 0; // 环形缓冲区满了就覆盖之前的数据
    }

    // 如果收到回车符，说明一句话接收完成了，设置完成标志
    if (received_char == '\r')
    {
      UART_RX_STA |= 0x8000;  // 设置接收完成标志
      UART_RX_STA &= ~0x4000; // 清除回车符标志（如果之前有的话）
    }
  }
  else if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
  {
    // 收到空闲中断，说明一段话接收完成了（可能是回车符结尾，也可能不是），设置完成标志
    UART_RX_STA |= 0x8000;                          // 设置接收完成标志
    UART_RX_STA &= ~0x4000;                         // 清除回车符标志（如果之前有的话）
    usart1_idle_flag = 1;                           // 设置为空闲标记
    USART_ClearITPendingBit(USART1, USART_IT_IDLE); // 清除空闲中断标志位
  }
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/**
 * @}
 */

/**
 * @}
 */

// 中断处理函数，函数名到startup_stm32f10x_hd.s中找
void EXTI0_IRQHandler(void)
{
  // 消除抖动，这种操作不建议，会卡住CPU，真正的消抖应该在中断里打标记，在主循环里处理
  // delay_ms(300000);

  // 最佳实践：可以用定时器来消抖
  if (EXTI_GetFlagStatus(EXTI_Line0))
  {
    systick_start(key_delay_ms); // 启动定时器，开始计时; 间隔小点，精确点
  }

  // TODO 如果是多个IO共用的中断处理函数，则第一步需要判断是哪一个IO触发的中断
  // 类似这种例子
  // 1. 判断是否是 PA8 触发的（读取 PA8 当前是否为低电平）
  // if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_RESET)
  // {
  // 	// 执行 PA8 对应的按键逻辑
  // 	LED_G_Toggle();
  // }

  // // 2. 判断是否是 PB9 触发的（读取 PB9 当前是否为低电平）
  // if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == Bit_RESET)
  // {
  // 	// 执行 PB9 对应的按键逻辑
  // 	LED_B_Toggle();
  // }
  // 清除标识位（参考数据手册中的流程图）
  EXTI_ClearITPendingBit(EXTI_Line0);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
