/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：user_conf.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _USER_CONF_H_
#define _USER_CONF_H_

#define MSG_BUF_MAX					8	/* 消息缓冲数量 */

#define BUF_CHECK_SIZE				32	/* 消息过滤 buffer 大小 */

#define GET_UID_CNT(Byte)		(96 / (Byte * 8))
#define UID_BASE_ADDR			(0x1FFFF7E8)

#define UID_CHECK_ADDR			(0x0800F800)
#define DATA_SAVE_ADDR  		(0x0800F000)
#define DATA_SAVE_HEAD 			(0xA5A5)

/*
 * 8 * 8矩阵
 */

#define KEY_X_CNT		9				/* 键盘扫描列 */
#define KEY_Y_CNT		12				/* 键盘扫描行 power */
#define KEY_SCAN_CNT	8				/* 消抖扫描次数 */
#define KEY_MIX_MAX		8				/* 单次支持的最大扫描个数 */

#define KEY_BUF_MAX		8				/* 键盘消息缓冲数量 */

#define LED_X_CNT		16				/* LED扫描列 */
#define LED_Y_CNT		12				/* LED扫描行 power */

#define MAX_Y_CNT		(KEY_Y_CNT > LED_Y_CNT ? KEY_Y_CNT : LED_Y_CNT)

//#define SYNC_MAX_TIME	(KEY_SCAN_CNT * 2 * KEY_BUF_MAX) /* 最大同步时间 单位 ms */
#define SYNC_MAX_TIME	(10 * 1000)	/* 10s */

#define CAM_ADDR_MAX						8
#define CODE_SWITCH_MAX						1
#define CODE_SWITCH1_MAX_VALUE				15
#define CODE_SWITCH2_MAX_VALUE				64
#define CODE_SWITCH3_MAX_VALUE				64
#define CODE_SWITCH4_MAX_VALUE				64

#define CODE_SWITCH1_REVERSE	1
#define CODE_SWITCH2_REVERSE	1
#define CODE_SWITCH3_REVERSE	1
#define CODE_SWITCH4_REVERSE	1


#endif

