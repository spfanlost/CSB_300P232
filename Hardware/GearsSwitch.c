
/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：GearsSwitch.C
  * 描    述：文件
  * 创 建 者: 余萌
  * 创建日期: 
  * 修 改 者:             
  * 修改日期: 
  * 版    本: V1.0.0
  ******************************************************************************
  * attention
  *	继电器档位
  ******************************************************************************
  */
#include "bsp.h"

uint16_t GearsValue = 1;//全局变量在.h文件引用

void SetGear(uint8_t GValue);

void bsp_Relay_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
	SetGear(GearsValue);
}
/********************************************************************************************************
  * @函数名称	设置档位函数
  * @函数说明   
  * @输入参数   无
  * @输出参数   范围是1、2、3
  * @返回参数   无
*********************************************************************************************************
*/

void SetGear(uint8_t GValue)
{
	
	switch(GValue)
		{
			case 1:
				GPIO_ResetBits(GPIOA,GPIO_Pin_11);
				GPIO_ResetBits(GPIOA,GPIO_Pin_12);
				break;
			case 2:
				GPIO_ResetBits(GPIOA,GPIO_Pin_12); 
				GPIO_SetBits(GPIOA,GPIO_Pin_11);
				break;
			case 3:
				GPIO_ResetBits(GPIOA,GPIO_Pin_11); 
				GPIO_SetBits(GPIOA,GPIO_Pin_12);
				break;
			case 4:
				break;
			default:break;
		}
}

/********************************************************************************************************
  * @函数名称	探测当前档位函数
  * @函数说明   
  * @输入参数   无
  * @输出参数   范围是1、2、3
  * @返回参数   无
*********************************************************************************************************
*/
uint16_t DetectGear(void)
{
	uint16_t gear = 1;
	
	if((0      == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_11))&&(0 == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_12)))
	{
	gear       = 1;
	
	}
	else if((1 == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_11))&&(0 == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_12)))
	{
	gear       = 2;
	}
	else if((0 == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_11))&&(1 == GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_12)))
	{
	gear       = 3;
	}
	
	return gear;

}
/********************************************************************************************************
  * @函数名称		自动设置档位函数
  * @函数说明   
  * @输入参数   无
  * @输出参数   范围是1、2、3
  * @返回参数   无
*********************************************************************************************************
*/
//数据暂定的
uint16_t RefreshGear(void)
{
	uint16_t Gear = 0;
	extern uint8_t GearDataFlag;
	extern float   GVotage;
	Gear = DetectGear();
	switch(Gear)	
	{
    	case 1:
			if(GVotage <= 1.187f)GearsValue=2; 
			else GearsValue=1; 
    		break;
    	case 2:
			if(GVotage <= 0.67f) GearsValue=3; 
			else if(GVotage >= 2.9f) GearsValue=1; 
			     else GearsValue=2; 
    		break;
    	case 3:
			if(GVotage >= 1.6f) GearsValue=2; 
			else GearsValue=3; 
			break;
    	default: GearsValue=1; 
    		break;
    }
	if(Gear != GearsValue)
	{
		GearDataFlag	= 1;	
		if(WM_IsWindow(WinPara.hWinTwo)) 	//判断窗口是否有效
		{
			WM_SendMessageNoPara(WinPara.hWinTwo,MY_MSG_ADD_GDATA);		
		}
	}
	SetGear(GearsValue);
	if(WM_IsWindow(WinPara.hWinStatus)) 	//判断窗口是否有效
	{
		WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_GAUTO);		
	}
	return GearsValue;
}










