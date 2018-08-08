/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_switch.h
* 摘要: PWM控制头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _PWM_H_
#define _PWM_H_
#include "stm32f10x_conf.h"

#define MCU_PWM1							GPIO_Pin_6
#define MCU_PWM2							GPIO_Pin_7
#define MCU_PWM_PORT 						GPIOA

#define MCU_PWM3							GPIO_Pin_0
#define MCU_PWM4							GPIO_Pin_1
#define MCU_PWM3_PORT 						GPIOB
#define MCU_PWM4_PORT 						GPIOB

#define MCU_PWM5							GPIO_Pin_13
#define MCU_PWM5_PORT 						GPIOD

#define MCU_PWM_TIMER 						TIM3

#define MCU_PWM1_CCR						MCU_PWM_TIMER->CCR1
#define MCU_PWM2_CCR						MCU_PWM_TIMER->CCR2
#define MCU_PWM3_CCR						MCU_PWM_TIMER->CCR3
#define MCU_PWM4_CCR						MCU_PWM_TIMER->CCR4

#define MCU_PWM1_OCInit						TIM_OC1Init
#define MCU_PWM2_OCInit						TIM_OC2Init
#define MCU_PWM3_OCInit						TIM_OC3Init
#define MCU_PWM4_OCInit						TIM_OC4Init

#define MCU_PWM1_OCPreloadConfig			TIM_OC1PreloadConfig
#define MCU_PWM2_OCPreloadConfig			TIM_OC2PreloadConfig
#define MCU_PWM3_OCPreloadConfig			TIM_OC3PreloadConfig
#define MCU_PWM4_OCPreloadConfig			TIM_OC4PreloadConfig


#define ENABLE_PWM_TIMER()		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)

#define PWM_RESOLUTION			256


enum
{
	_PWM_Channel_Red,
	_PWM_Channel_Low,
	_PWM_Channel_Green,
	_PWM_Channel_Orange,
	
	
	_PWM_Channel_Reserved,
};


void PWMCtrlInit(void);

void RedressLedLight(s32 s32Channel);

extern u16 g_u16LedOnLight;
extern u16 g_u16LedOffLight;
extern u16 g_u16LedOn1Light;
extern u16 g_u16LedOn2Light;


#endif
