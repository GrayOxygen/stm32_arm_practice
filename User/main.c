/**
 ******************************************************************************
 * @file    GPIO/IOToggle/main.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main program body.
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
#include "stm32f10x.h"
#include "Beep.h"
#include "key.h"
#include "sysclk.h"
#include "systick.h"
#include "led.h"
#include <math.h>
#include "my_timer.h"
#include "my_usart.h"
#include "string.h"

// 软件延时，不准确的时间，真正的时间要根据时钟频率来计数
// void delay_ms(unsigned int count)
// {
//   while (count--)
//     ;
// }

void test_beep_by_register()
{
  // 初始化beep，不响
  beep_init();
  while (1)
  {
    // 通过ODR更新输出值：先读，再改，最后才写入，线程不安全
    // 让蜂鸣器响
    GPIOC->ODR &= ~(1 << 9); // ODR寄存器的第9位置0，BEEP导通，响
    // 延时
    delay_ms(300);
    // 让蜂鸣器不响
    GPIOC->ODR |= (1 << 9); // ODR寄存器，设置第9位置1，BEEP截止，不响
    // 延时
    delay_ms(300);
  }
}

void test_beep_safe_by_register()
{
  // 初始化beep，不响
  beep_init();
  while (1)
  {
    // 通过BSRR(bit set/reset register 置1或置0)和BRR(bit reset register 置0)更新值：都是w的原子操作，线程安全
    // 让蜂鸣器响
    GPIOC->BRR = (1 << 9); // BRR寄存器的第9位置1，电平为低电平，BEEP导通，响
    // 也可以用BSRR设为低电平
    // GPIOC->BSRR=(1<<(9+16)); // BSRR寄存器的第25位置1，电平为低电平，BEEP导通，响
    // 延时
    delay_ms(300);
    // 让蜂鸣器不响
    GPIOC->BSRR = (1 << 9); // BSRR寄存器的第9位置1，BEEP截止，不响（BSRR低位15:0是set，高位31:16是reset）
    // 延时
    delay_ms(300);
  }
}

void test_beep_by_std()
{
  beep_init_std();
  while (1)
  {
    // 让蜂鸣器响
    GPIO_ResetBits(GPIOC, GPIO_Pin_9); // GPIO_ResetBits函数会将GPIO_Pin_9位置0，BEEP导通，响
    // 延时
    delay_ms(300);
    // 让蜂鸣器不响
    GPIO_SetBits(GPIOC, GPIO_Pin_9); // GPIO_SetBits函数会将GPIO_Pin_9位置1，BEEP截止，不响
    // 延时
    delay_ms(300);
  }
}

void test_led_by_bitband()
{
  LED_Init();
  while (1)
  {
    LED_R_On();
    // 延时
    delay_ms(300);
    LED_R_Off();
    // 延时
    delay_ms(300);

    LED_G_On();
    // 延时
    delay_ms(300);
    LED_G_Off();
    // 延时
    delay_ms(300);

    LED_B_On();
    // 延时
    delay_ms(300);
    LED_B_Off();
    // 延时
    delay_ms(300);
  }
}

// 1. 定义查找表：360个色相，每个色相存3个值(R, G, B)，所以大小是 360 * 3
static uint8_t hsv_rgb_lut[360 * 3];

// 2. 初始化查找表
void init_hsv_lut(void)
{
  uint16_t h;
  for (h = 0; h < 360; h++)
  {
    uint8_t region = h / 60;
    uint16_t remainder = (h % 60) * 4;

    uint8_t p = 0;
    uint16_t temp_q = (255 * remainder) >> 8;
    uint8_t q = (255 * (255 - temp_q)) >> 8;

    uint16_t temp_t_base = (255 * (255 - remainder)) >> 8;
    uint8_t t = (255 * (255 - temp_t_base)) >> 8;

    // 使用指针偏移的方式分别存入 R, G, B (等价于 [h], [h], [h])
    switch (region)
    {
    case 0:
      *(hsv_rgb_lut + h * 3) = 255;
      *(hsv_rgb_lut + h * 3 + 1) = t;
      *(hsv_rgb_lut + h * 3 + 2) = p;
      break;
    case 1:
      *(hsv_rgb_lut + h * 3) = q;
      *(hsv_rgb_lut + h * 3 + 1) = 255;
      *(hsv_rgb_lut + h * 3 + 2) = p;
      break;
    case 2:
      *(hsv_rgb_lut + h * 3) = p;
      *(hsv_rgb_lut + h * 3 + 1) = 255;
      *(hsv_rgb_lut + h * 3 + 2) = t;
      break;
    case 3:
      *(hsv_rgb_lut + h * 3) = p;
      *(hsv_rgb_lut + h * 3 + 1) = q;
      *(hsv_rgb_lut + h * 3 + 2) = 255;
      break;
    case 4:
      *(hsv_rgb_lut + h * 3) = t;
      *(hsv_rgb_lut + h * 3 + 1) = p;
      *(hsv_rgb_lut + h * 3 + 2) = 255;
      break;
    default:
      *(hsv_rgb_lut + h * 3) = 255;
      *(hsv_rgb_lut + h * 3 + 1) = p;
      *(hsv_rgb_lut + h * 3 + 2) = q;
      break;
    }
  }
}

// 3. 彩虹呼吸灯主函数（带独立色彩权重补偿）
void test_led_rainbow_breath()
{
  LED_Init();
  LED_PWM_Init();
  init_hsv_lut();

  // 【核心修改】针对你这颗“偏科”灯珠的极端补偿参数
  // 红灯太暗？我们直接给它 5 倍的亮度增益（255 * 5 = 1275，远超正常值）！
  // 绿光和蓝光太强？我们把它们的权重压到 0.05（也就是只有原本的 5% 亮度）！
  float r_gain = 7.73;
  float g_ratio = 3.73;
  float b_ratio = 2.11;

  uint16_t hue = 0;

  while (1)
  {
    // 1. 从查找表取出基础 RGB 值
    uint8_t base_r = *(hsv_rgb_lut + hue * 3);
    uint8_t base_g = *(hsv_rgb_lut + hue * 3 + 1);
    uint8_t base_b = *(hsv_rgb_lut + hue * 3 + 2);

    // 2. 加入呼吸效果（正弦波平滑过渡）  
    float breath = (sinf(hue * 3.14159f / 180.0f) + 1.0f) / 2.0f;
    
    // 3. 【核心修改】应用极端的权重和增益
    // 红色：基础值 * 5倍增益 * 呼吸因子
    uint16_t final_r = (uint16_t)(base_r * r_gain * breath);
    // 绿/蓝：基础值 * 0.05超低权重 * 呼吸因子
    uint16_t final_g = (uint16_t)(base_g * g_ratio * breath);
    uint16_t final_b = (uint16_t)(base_b * b_ratio * breath);

    // 4. 限制最大值不超过 255（防止 PWM 溢出）
    if (final_r > 255)
      final_r = 255;
    if (final_g > 255)
      final_g = 255;
    if (final_b > 255)
      final_b = 255;

    // 5. 输出颜色
    LED_Set_Color(final_r, final_g, final_b);

    hue++;
    if (hue >= 360)
    {
      hue = 0;
    }

    delay_ms(3);

    // 目前是这样，每个我都单独试过了，rgb对应了
    // 草绿色，像是#00FF00色彩  when  only r=255
    // 蓝色，#0000FF  when only g =255
    // 青绿色，#40E0D0  when only b = 255
    // LED_Set_Color(0, 255, 0); // 绿，蓝，银绿色
    // delay_ms(500000);
  }
}

void test_fast_spectrum()
{
  LED_PWM_Init();
  init_hsv_lut();
  uint16_t hue = 0;

  while (1)
  {
    uint8_t r = *(hsv_rgb_lut + hue * 3);
    uint8_t g = *(hsv_rgb_lut + hue * 3 + 1);
    uint8_t b = *(hsv_rgb_lut + hue * 3 + 2);

    LED_Set_Color(r, g, b);

    // 每次跳跃 7 个色相，打破常规顺序，让颜色看起来更多变
    hue = (hue + 7) % 360;

    delay_ms(10000); // 极短的延时，产生快速闪变的效果
  }
}

void longShortPressWithTimerInputCapture()
{

  // 测试定时器：输入捕获
  tim5_ch1_init(10000, 72); // 10ms周期，1MHz的计数频率
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  // 外设初始化
  beep_init_std();
  remap_init();
  key_init();
  tim5_ch1_init(0xffff, 72); // 最大记录4.1s，1MHz的计数频率
  uint32_t temp = 0;
  while (1)
  {
    if (catch_status & 0x80)
    { // 完成捕获
      // 总时长 = 溢出的圈数 × 每圈的时长（65536） + 最后一圈没跑完的零头
      temp = catch_status & 0x3f; // 溢出次数
      temp *= 65536;              // 16位定时器，可以记录65536微秒，也就是65.536毫秒
      temp += catch_tim5_reg;     // temp为总共的计数，每一个数是1微妙，加上最后一圈（可能不够溢出），就得到了按键按下的总时间
      catch_status = 0;           // 清除状态，准备下一次捕获
      if (temp >= 300000)
      { // 长按超过3秒，触发蜂鸣器响
        BEEP_TOGGLE();
      }
    }
  }
}

// 发送一个完整的字符串（支持 UTF-8 字节数组）
void usart1_putstr(uint8_t *str, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++)
  {
    usart1_putchar(str[i]);
  }
}

// 测试串口通信
void testUSART()
{

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  // 外设初始化
  beep_init_std();
  remap_init();
  key_init();
  usart1_init();
  while (1)
  {
    uint16_t received = usart1_getchar(); // 接收数据

    char buffer[50];                                                  // 申请一块小内存当缓冲区
    sprintf(buffer, "%s%c", "Wow. 收到了你的消息：", (char)received); // 格式化拼接字符串
    usart1_putstr((uint8_t *)buffer, strlen(buffer));                 // 统一发送

    // 3. 最后发一个回车换行，让串口助手的显示更整齐
    usart1_putchar('\r'); // 回车
    usart1_putchar('\n'); // 换行
  }
}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void)
{
  // 测试不同方式的BEEP触发
  // 1，寄存器方式
  // test_beep_by_register();
  // test_beep_safe_by_register();
  // 2，标准库方式
  // test_beep_by_std();

  // 点灯
  // test_led_by_bitband();
  test_led_rainbow_breath();

  // 	// 系统时钟，PLL锁相环练习
  // 	RCC_DeInit();// 禁用系统时钟
  // 	sysclk_init();// 使用自定义初始化时钟

  // // 中断
  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  // // 外设初始化
  // beep_init_std();
  // remap_init();
  // key_init();
  // while (1)
  // { // 按下按键，走中断逻辑，可以触发beep
  //   key_scan();
  // }

  // 输入捕获练习：长按短按识别，基于定时器输入捕获功能实现
  // longShortPressWithTimerInputCapture();

  // 测试串口通信
  // testUSART();
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
