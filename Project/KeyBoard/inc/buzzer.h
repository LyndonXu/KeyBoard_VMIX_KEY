/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：buzzer.h
* 摘要: 重要的驱动
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年05月08日
*******************************************************************************/
#ifndef _BUZZER_H_
#define _BUZZER_H_
typedef enum _tagEmBuzzerState
{
	_Buz_Close,
	_Buz_Msg_Sync_Finish,
	_Buz_Msg_Sync_Err,
	_Buz_Msg_Set_Preset,
	_Buz_Msg_Del_Preset,
}EmBuzzerState;


typedef struct _tagStBuzzerCtrl
{
	bool boIsBuzzing;
	bool boCurIsBuzzing;
	u8 u8BuzzDestCnt;
	u8 u8BuzzCnt;
	u16 u16BuzzTime;
	u32 u32BuzzStartTime;
}StBuzzerCtrl;

#define BUZZ_PIN			GPIO_Pin_8
#define BUZZ_PIN_PORT		GPIOA

#define BUZZ_ENABLE()		TIM_Cmd(TIM1,ENABLE); BuzzerGPIOOpen()
#define BUZZ_DISABLE()		TIM_Cmd(TIM1,DISABLE); BuzzerGPIOClose();
void BuzzerInit(void);
void StartBuzzer(EmBuzzerState emState);
void FlushBuzzer(void);


#endif

