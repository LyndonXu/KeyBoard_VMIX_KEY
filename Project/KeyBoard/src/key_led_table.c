#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "key_led.h"
#include "key_led_table.h"

u8 g_u8KeyTable[KEY_Y_CNT][KEY_X_CNT] = 
{
	{
		_Key_QuickPlay2,
		_Key_FullScreen,
		_Key_MultiCorder,
		_Key_PlayList,
		_Key_StartExternal,
		_Key_Recording,
		_Key_Stream,
	},	/* 1 */
	{
		_Key_QuickPlay2,
		_Key_QuickPlay3,
		_Key_QuickPlay4,
		_Key_Loop1,	
		_Key_Loop2,			
		_Key_Loop3,			
		_Key_Loop4,		
	},	/* 2 */
	{
		_Key_Overlay2,
		_Key_Overlay4,
		_Key_Overlay6,
		_Key_Overlay8,
		_Key_Overlay10,
		_Key_Overlay12,		
		_Key_Play2,
		_Key_Play4,

	},	/* 3 */
	{
		_Key_Overlay1,				
		_Key_Overlay3,
		_Key_Overlay5,
		_Key_Overlay7,
		_Key_Overlay9,
		_Key_Overlay11,
		_Key_Play1,
		_Key_Play3,

	},	/* 4 */
	{
		_Key_PGM_1,
		_Key_PGM_2,
		_Key_PGM_3,
		_Key_PGM_4,
		_Key_PGM_5,
		_Key_PGM_6,
		_Key_PGM_7,
		_Key_PGM_8,

	},	/* 5 */
	{
		_Key_PVW_1,
		_Key_PVW_2,
		_Key_PVW_3,
		_Key_PVW_4,
		_Key_PVW_5,
		_Key_PVW_6,
		_Key_PVW_7,
		_Key_PVW_8,
	
	}, /* 6 */
	{
		_Key_PGM_9,
		_Key_PGM_10,
		_Key_PGM_11,
		_Key_PGM_12,		
	}, /* 7 */
	{
		_Key_PVW_9,
		_Key_PVW_10,
		_Key_PVW_11,
		_Key_PVW_12,		
	}, /* 8 */
	{
		0,
		_Key_Transition2,
		_Key_Transition4,
		0,
		0, 0, 0, 0,
		_Key_PowerDown,
	}, /* 9 */
	{
		0,
		_Key_Transition1,
		_Key_Transition3,
	}, /* 10 */
	
	{
		_Key_Dsk1,
		_Key_Dsk2,
		_Key_Dsk3,
		_Key_Dsk4,
	}, /* 11 */
	
	{
		_Key_FTB,
		_Key_QuickPlay,
		_Key_Effect_Ctrl_Cut,
		_Key_Effect_Ctrl_Take,
	}, /* 12 */
};

/* dp, g, f, e, d, c, b, a */
const u8 g_u8LED7Code[] = 
{
	0x3F,		// 0
	0x06,		// 1
	0x5B,		// 2
	0x4F,		// 3
	0x66,		// 4
	0x6D,		// 5
	0x7D,		// 6
	0x07,		// 7
	0x7F,		// 8
	0x6F,		// 9
	0x77,		// A
	0x7C,		// B
	0x39,		// C
	0x5E,		// D
	0x79,		// E
	0x71,		// F
	0x40,		// -
};
 

const u16 g_u16CamAddrLoc[CAM_ADDR_MAX] = 
{
	0,
};

