#include "Beep.h"
#include "stm32f10x_usart.h"
// 使用寄存器的方式启动蜂鸣器
void beep_init(void)
{
	// 先看原理图，确定PC9连接BEEP，再看数据手册，确定PC9在APB2总线上
	// 1，开启总线时钟（RCC是时钟控制，ENR是enable register，查看数据手册，确定第4位要设置为1，是开启）
	RCC->APB2ENR |= (1 << 4); // 使能GPIOC的时钟

	// 注意：
	// RCC (复位和时钟控制)：相当于整栋大楼的总配电室3。
	// APB2 总线：相当于大楼里的某一条主供电线路（比如 3 楼的供电干线）2。
	// GPIOC：相当于这条供电线路上的一个完整房间（C 房间）2。
	// PC9：相当于 C 房间里的第 9 号插座1。
	// 当你执行 RCC->APB2ENR |= (1 << 4); 时，你做的动作是：去总配电室，把“C 房间”的总电闸给推上去了。

	// 2，设置PC9的输入输出模式（查看数据手册，CRL管的0-7号引脚，CRH管的8-15号引脚，[8:4]位为mode和频率）
	// 触发BEEP，为发送数据，则设置推挽输出模式，即0011（11表示选择了最大的频率，更快）
	GPIOC->CRH &= ~(0xF << 4); // 先清零，再配置
	GPIOC->CRH |= (0x3 << 4);  // 设置为推挽，输出，即00 11
	// 注意：只要是引脚Pinout，就要思考是否要设置输入输出模式，本质上，就是是否发送信号（电流流动）
	// 芯片的引脚 = 必须干固定活的“基础设施引脚”（电源、时钟、复位等） + 可以自由配置的“GPIO引脚”（这些 GPIO 还能兼职做各种通信和控制外设）。

	// 默认关闭蜂鸣器
	BEEP_OFF();
}

// 使用标准库的方式输出GPIO
void beep_init_std(void)
{
	GPIO_InitTypeDef gpio_instruct;
	// 1，开启GPIOC所在总线的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	// 2，初始化GPIOC外设
	gpio_instruct.GPIO_Pin = GPIO_Pin_9;
	gpio_instruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_instruct.GPIO_Mode = GPIO_Mode_Out_PP; // push-pull output

	GPIO_Init(GPIOC, &gpio_instruct);
	// 3，设置默认状态下蜂鸣器的状态：默认为1，即不响（因为原理图显示BEEP是个PNP三极管，低电平才导通）
	GPIO_SetBits(GPIOC, GPIO_Pin_9);
}

/**
 * PB4:主功能复位后NJTRST;默认复用功能 SPI3_MISO；重定义：PB4/TIM3_CH1/SPI1_MISO
 *
 * 重映射：当一个pinout的io被占用，如PB4，main function after reset是JNTRST，复用为一个普通的GPIO使用
 */
void remap_init(void)
{
	GPIO_InitTypeDef gpio_instruct;
	// 1，使能时钟：总线外设时钟，外设复用；AFIO（重映射必须开启），重映射后使用的引脚所在的GPIO端口（仍是自己）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	// 重映射：查看reference知道，只要大于000，这里选010的情况，保证JNTRST可用，即恢复PB4的功能，作为一个普通的GPIO使用
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE); // PB4当做一个GPIO普通使用
	
	// 2，设置重映射后的GPIOC
	gpio_instruct.GPIO_Pin = GPIO_Pin_4;
	gpio_instruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_instruct.GPIO_Mode = GPIO_Mode_Out_PP; // push-pull output

	GPIO_Init(GPIOC, &gpio_instruct);

	// 3、设置默认状态下蜂鸣器的状态
	// void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
	// void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
}

/**
 * PA9：主功能（复位后）PA9；默认复用功能：USART1_TX(7) TIM1_CH2(7)；重定义： 无
 * PA10：主功能（复位后）PA10；默认复用功能：USART1_RX(7) / TIM1_CH3(7)；重定义： 无
 * PB6: 主功能（复位后） PB6；默认复用功能：I2C1_SCL(7)/TIM4_CH1(7)；重定义：USART1_TX
 * PB7: 主功能（复位后） PB7；默认复用功能：I2C1_SDA(7)/FSMC_NADV/TIM4_CH2(7)；重定义：USART1_RX
 *
 * 把 USART1 这个外设的“嘴巴（TX）”和“耳朵（RX）”，从默认的 PA9 和 PA10 两个引脚上，强行挪到了 PB6 和 PB7 上去
 */
void remap_example(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 1. 开启时钟：必须开启 GPIOB（重映射的piniout）、USART1（PA9 PA10的复用） 以及 AFIO（重映射必配）的时钟！
	// （AFIO 时钟就是调度中心的电源，不开它没法改路标）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

	// 2. 拨动 AFIO 路标：将 USART1 映射到 PB6/PB7（相当于PA9 PA10恢复成为了普通引脚，不过时钟没开启A，所以A没在被使用）
	// 标准库封装好了这个函数，底层其实就是去设置 AFIO->MAPR 寄存器的第2位
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

	// 3. 配置重映射后的引脚 PB6 (TX是输出的) 为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 4. 配置重映射后的引脚 PB7 (RX是读取的) 为浮空输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 因为RX是读所以设置为输入模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 5. 配置 USART1 的参数（波特率等，和平时写串口一样）
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	// 6. 开启 USART1
	USART_Cmd(USART1, ENABLE);
}
