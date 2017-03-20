/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：sys.c
  * 描    述：文件
  * 创 建 者: 余萌
  * 创建日期: 
  * 修 改 者:             
  * 修改日期: 
  * 版    本: V1.0.0
  ******************************************************************************
  * attention
  *	系统配置函数
  ******************************************************************************
  */

#include "sys.h"

//看门狗初始化
void IWDG_Init(void) 
{	
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //使能对寄存器IWDG_PR和IWDG_RLR的写操作
	IWDG->PR = IWDG_Prescaler_64;
	IWDG->RLR = 0xfff;	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
	IWDG_Enable();  //使能IWDG
}
//喂独立看门狗
void IWDG_Feed(void)
{   
 	IWDG_ReloadCounter();//reload										   
}

/*禁止jtag 打开sw两线下载 */
void JATG_Disable_SW_Enable(void)
{
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	 GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);   
	 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);   //关闭jtaq模式  打开sw模式  本款产品只提供sw两线下载以及串口下载 
}
