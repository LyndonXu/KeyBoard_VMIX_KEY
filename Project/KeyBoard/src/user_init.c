/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：user_init.c
* 摘要: 一些简单的初始化
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_init.h"


/*
 * 使能所有外设引脚时钟
 * 输入: 无
 * 输出: 无
 */
void PeripheralPinClkEnable(void)
{
	/* enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	/* Enable all the GPIO and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | 
	                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
}

/* jtag and osc32 */
void OpenSpecialGPIO(void)
{
	/* the main function of PC13, PC14, PC15 is GPIO */

	/* JTAG-DP Disabled and SW-DP Enabled */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void SysTickInit(void) /* 1ms */
{
	SysTick_Config(SystemCoreClock / 1000);
}

void UART3Init(u32 u32Bandrate)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	USART_Cmd(USART3, DISABLE);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = u32Bandrate;
	USART_Init(USART3, &USART_InitStructure);
	USART_Cmd(USART3, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void UART2Init(u32 u32Bandrate)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	USART_Cmd(USART2, DISABLE);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = u32Bandrate;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
