#ifndef _KEY_LED_TABLE_H_
#define _KEY_LED_TABLE_H_
#include "stm32f10x_conf.h"
#include "user_conf.h"

#define LOC(x, y) 		((((x - 1) & 0xFF) << 8) | ((y - 1) & 0xFF))  	/* 高8 位X 的位置，低8 位Y 的位置 */
#define GET_X(loc)		((loc >> 8) & 0xFF)
#define GET_Y(loc)		(loc & 0xFF)
#define GET_XY(loc) 	GET_X(loc), GET_Y(loc)

extern u8 g_u8KeyTable[KEY_Y_CNT][KEY_X_CNT];
enum 
{
	_Key_Ctrl_Begin = 1,
	
	_Key_QuickPlay1 = _Key_Ctrl_Begin,
	_Key_QuickPlay2,
	_Key_QuickPlay3,
	_Key_QuickPlay4,
	
	_Key_Loop1,	
	_Key_Loop2,			
	_Key_Loop3,			
	_Key_Loop4,	

	_Key_FullScreen,
	_Key_MultiCorder,
	_Key_PlayList,
	_Key_StartExternal,
	_Key_Recording,
	_Key_Stream,
	
	
	/* ABUS */
	_Key_PGM_1,				
	_Key_PGM_2,
	_Key_PGM_3,
	_Key_PGM_4,
	_Key_PGM_5,
	_Key_PGM_6,
	_Key_PGM_7,
	_Key_PGM_8,
	_Key_PGM_9,
	_Key_PGM_10,
	_Key_PGM_11,
	_Key_PGM_12,		

	_Key_PVW_1,
	_Key_PVW_2,
	_Key_PVW_3,
	_Key_PVW_4,
	_Key_PVW_5,
	_Key_PVW_6,
	_Key_PVW_7,
	_Key_PVW_8,
	_Key_PVW_9,
	_Key_PVW_10,
	_Key_PVW_11,
	_Key_PVW_12,		

	_Key_Effect_Ctrl_Take,
	_Key_Effect_Ctrl_Cut,
	
	_Key_Overlay1,				
	_Key_Overlay2,
	_Key_Overlay3,
	_Key_Overlay4,
	_Key_Overlay5,
	_Key_Overlay6,
	_Key_Overlay7,
	_Key_Overlay8,
	_Key_Overlay9,
	_Key_Overlay10,
	_Key_Overlay11,
	_Key_Overlay12,		

	
	_Key_FTB,
	_Key_QuickPlay,


	_Key_Dsk1,
	_Key_Dsk2,
	_Key_Dsk3,
	_Key_Dsk4,

	_Key_Play1,
	_Key_Play2,
	_Key_Play3,
	_Key_Play4,

	_Key_Transition1,
	_Key_Transition2,
	_Key_Transition3,
	_Key_Transition4,

	_Key_Ctrl_Reserved_Inner1,

	_Key_PowerDown = _Key_Ctrl_Reserved_Inner1,
	_Key_Ctrl_Reserved,
};


enum 
{
	_Led_QuickPlay1 = LOC(1, 2),
	_Led_QuickPlay2 = LOC(1, 1),
	_Led_QuickPlay3 = LOC(3, 2),
	_Led_QuickPlay4 = LOC(5, 2),
	
	_Led_Loop1 = LOC(7, 2),	
	_Led_Loop2 = LOC(9, 2),			
	_Led_Loop3 = LOC(11, 2),			
	_Led_Loop4 = LOC(13, 2),	

	_Led_FullScreen = LOC(3, 1),
	_Led_MultiCorder = LOC(5, 1),
	_Led_PlayList = LOC(7, 1),
	_Led_StartExternal = LOC(9, 1),
	_Led_Recording = LOC(11, 1),
	_Led_Stream = LOC(13, 1),
	
	
	/* ABUS */
	_Led_PGM_1 = LOC(1, 5),				
	_Led_PGM_2 = LOC(3, 5),
	_Led_PGM_3 = LOC(5, 5),
	_Led_PGM_4 = LOC(7, 5),
	_Led_PGM_5 = LOC(9, 5),
	_Led_PGM_6 = LOC(11, 5),
	_Led_PGM_7 = LOC(13, 5),
	_Led_PGM_8 = LOC(15, 5),
	_Led_PGM_9 = LOC(1, 7),
	_Led_PGM_10 = LOC(3, 7),
	_Led_PGM_11 = LOC(5, 7),
	_Led_PGM_12 = LOC(7, 7),		

	_Led_PVW_1 = LOC(1, 6),
	_Led_PVW_2 = LOC(3, 6),
	_Led_PVW_3 = LOC(5, 6),
	_Led_PVW_4 = LOC(7, 6),
	_Led_PVW_5 = LOC(9, 6),
	_Led_PVW_6 = LOC(11, 6),
	_Led_PVW_7 = LOC(13, 6),
	_Led_PVW_8 = LOC(15, 6),
	_Led_PVW_9 = LOC(1, 8),
	_Led_PVW_10 = LOC(3, 8),
	_Led_PVW_11 = LOC(5, 8),
	_Led_PVW_12 = LOC(7, 8),		

	_Led_Effect_Ctrl_Take = LOC(7, 12),
	_Led_Effect_Ctrl_Cut = LOC(5, 12),
	
	_Led_Overlay1 = LOC(1, 4),				
	_Led_Overlay2 = LOC(1, 3),
	_Led_Overlay3 = LOC(3, 4),
	_Led_Overlay4 = LOC(3, 3),
	_Led_Overlay5 = LOC(5, 4),
	_Led_Overlay6 = LOC(5, 3),
	_Led_Overlay7 = LOC(7, 4),
	_Led_Overlay8 = LOC(7, 3),
	_Led_Overlay9 = LOC(9, 4),
	_Led_Overlay10 = LOC(9, 3),
	_Led_Overlay11 = LOC(11, 4),
	_Led_Overlay12 = LOC(11, 3),		


	_Led_FTB = LOC(1, 12),
	_Led_QuickPlay = LOC(3, 12),


	_Led_Dsk1 = LOC(1, 11),
	_Led_Dsk2 = LOC(3, 11),
	_Led_Dsk3 = LOC(5, 11),
	_Led_Dsk4 = LOC(7 , 11),

	_Led_Play1 = LOC(13, 4),
	_Led_Play2 = LOC(13, 3),
	_Led_Play3 = LOC(15, 4),
	_Led_Play4 = LOC(15, 3),

	_Led_Transition1 = LOC(1, 10),
	_Led_Transition2 = LOC(1, 9),
	_Led_Transition3 = LOC(3, 10),
	_Led_Transition4 = LOC(3, 9),

	_Led_PowerDown = LOC(5, 9),
	_T_PushPod1 = LOC(7, 10),
	_T_PushPod2 = LOC(8, 10),
};



#endif

