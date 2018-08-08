/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：app_port.h
* 摘要: 重要的驱动
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年05月08日
*******************************************************************************/
#ifndef _APP_PORT_H_
#define _APP_PORT_H_

#define CRITICAL_METHOD		3


#if CRITICAL_METHOD == 3

int CPU_SR_Save(void);
void CPU_SR_Restore(int cpu_sr);

#define  USE_CRITICAL()		int cpu_sr = 0
#define  ENTER_CRITICAL()	{cpu_sr = CPU_SR_Save();}
#define  EXIT_CRITICAL()	{CPU_SR_Restore(cpu_sr);}

#elif CRITICAL_METHOD == 4

#define OS_MAX_SYSCALL_INTERRUPT_PRIORITY				0x2F			/* equivalent to 0x2F, or priority 2 */

void CPU_SetInterruptMask(void);
void CPU_ClearInterruptMask(void);


#define  USE_CRITICAL()
#define  ENTER_CRITICAL()	{CPU_SetInterruptMask();}
#define  EXIT_CRITICAL()	{CPU_ClearInterruptMask();}

#endif



#endif
