/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：adc_ctrl.h
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _ADC_CTRL_H_
#define _ADC_CTRL_H_
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "IOCtrl.h"

#define ROCK_X				GPIO_Pin_0
#define ROCK_Y				GPIO_Pin_1
#define ROCK_Z				GPIO_Pin_2
#define PUSH_ROD			GPIO_Pin_3

#define ROCK_X_CHANNEL			0
#define ROCK_Y_CHANNEL			1
#define ROCK_Z_CHANNEL			2
#define PUSH_ROD_CHANNEL		3
#define VOLUME_CHANNEL			4

#define ADC_CHANNEL_1			ADC_Channel_11
#define ADC_CHANNEL_2			ADC_Channel_10
#define ADC_CHANNEL_3			ADC_Channel_12
#define ADC_CHANNEL_4			ADC_Channel_13
#define ADC_CHANNEL_5			ADC_Channel_8

#define ADC_PIN_1				GPIO_Pin_0
#define ADC_PIN_2				GPIO_Pin_1
#define ADC_PIN_3				GPIO_Pin_2
#define ADC_PIN_4				GPIO_Pin_3
#define ADC_PIN_5				GPIO_Pin_0


#define ADC_PORT_1				GPIOC
#define ADC_PORT_2				GPIOC
#define ADC_PORT_3				GPIOC
#define ADC_PORT_4				GPIOC
#define ADC_PORT_5				GPIOB




#define ADC_GET_TOTAL				5
#define ADC_GET_CNT					16

#define PUSH_ROD_POS				3
#define PUSH_ROD_DIFF				5
#define PUSH_ROD_MAX_VALUE			119
#define PUSH_ROD_TIMES				8
#define PUSH_ROD_MIN_TIMES			3
#define PUSH_ROD_END				1590
#define PUSH_ROD_BEGIN				2550


#define BIG_ROCK_NEGATIVE_END			1460
#define BIG_ROCK_NEGATIVE_BEGIN			1843

#define BIG_ROCK_POSITIVE_END			2600
#define	BIG_ROCK_POSITIVE_BEGIN			2217

#define BIG_ROCK_MAX_VALUE				127
#define BIG_ROCK_TIMES					3

#define ROCK_NEGATIVE_BEGIN				0x980
#define ROCK_NEGATIVE_END				0xE80

#define	ROCK_POSITIVE_BEGIN				0x680
#define ROCK_POSITIVE_END				0x180

#define ROCK_MAX_VALUE					127
#define ROCK_TIMES						(0x500 / 127)

#define ROCK_X_NEGATIVE_DIR				0x02
#define ROCK_X_POSITIVE_DIR				0x01

#define ROCK_Y_NEGATIVE_DIR				0x08
#define ROCK_Y_POSITIVE_DIR				0x04

#define ROCK_Z_NEGATIVE_DIR				0x20
#define ROCK_Z_POSITIVE_DIR				0x10


#define VOLUME_POS					3
#define VOLUME_DIFF					30
#define VOLUME_MAX_VALUE			100
#define VOLUME_TIMES				40
#define VOLUME_MIN_TIMES			5
#define VOLUME_END					4048
#define VOLUME_BEGIN				48

typedef struct _tagStVolumeState
{
	u8 u8VolumeValue;
	u8 u8VolumeOldValue;
	u16 u16VolumeRealValue;
}StVolumeState;



typedef struct _tagStRockState
{
	u8 u8RockDir;		/* 摇杆方向 */
	u8 u8RockOldDir;	/* 摇杆方向 */
	
	u16 u16RockXValue;	/* 摇杆X值 */
	u16 u16RockYValue;	/* 摇杆Y值 */
	u16 u16RockZValue;	/* 摇杆Z值 */
	
	u16 u16RockXOldValue;	/* 摇杆X值 */
	u16 u16RockYOldValue;	/* 摇杆Y值 */
	u16 u16RockZOldValue;	/* 摇杆Z值 */

}StRockState;

typedef struct _tagStPushRodState
{
	u8 u8PushRodValue;		
	u8 u8PushRodOldValue;		
	u16 u16PushRodRealValue;
}StPushRodState;

extern bool g_boIsPushRodNeedReset;

extern u16 g_u16Times;
extern u16 g_u16UpLimit;
extern u16 g_u16DownLimit;

extern u16 g_u16VolumeTimes;
extern u16 g_u16VolumeUpLimit;
extern u16 g_u16VolumeDownLimit;

void RockPushRodInit(void);


u8 PushRodGetCurValue(void);
u8 VolumeGetCurValue(void);



void RockFlush(void);
void PushRodFlush(void);
void VolumeFlush(void);

bool RedressPushRodLimit (StIOFIFO *pFIFO);
void RedressVolumeLimit(void);

#endif
