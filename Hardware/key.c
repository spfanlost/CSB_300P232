/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：key.C
  * 描    述：文件
  * 创 建 者: 余萌
  * 创建日期: 2016.4.10
  * 修 改 者:             
  * 修改日期: 
  * 版    本: V1.0.0
  ******************************************************************************
  * updata:  16.4.10
  *	替换按键驱动为FIFO功能。
  *	实现按下，弹起，长按，连发，组合按键等功能。
  *
  * 修改需要注意
  *	key.h中：按键组合 KEY_COUNT KEY_FILTER_TIME KEY_LONG_TIME KEY_FIFO_SIZE
  *
  *
  ******************************************************************************
  */

#include "bsp.h"

static KEY_T s_tBtn[KEY_COUNT];
static KEY_FIFO_T s_tKey;		/* 按键FIFO变量,结构体 */

static void InitKeyVar(void);
static void InitKeyHard(void);
static void bsp_DetectKey(uint8_t i);

void bsp_OnOffKey_init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;// ON/OFF按键按下去为上升沿，故配置为下拉          
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);      
	
	EXTI_ClearITPendingBit(EXTI_Line1);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource1); 
	EXTI_InitStructure.EXTI_Line                         = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode                         = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger                      = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd                      = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel                   = EXTI1_IRQn;			 //按键件外部中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;					
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;							
	NVIC_Init(&NVIC_InitStructure); 		

}
void EXTI1_IRQHandler(void)
{
	static uint32_t power_temp=0;
	extern uint16_t BacklightTime; 
	extern uint16_t ShutdownCount ;              
	extern uint16_t ShutdownFlag;
	GPIO_InitTypeDef GPIO_InitStructure;
	bsp_delay_us(1000);			
	if(EXTI_GetFlagStatus(EXTI_Line1))
	{
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1))	 
		{
			power_temp++;
		}
		else                           //一旦按键放开 即可清除中断标志和关机计数
		{
			power_temp=0;
			BacklightTime = 0;				//清零背光变暗计时
			ShutdownFlag = 0;				//清零关机计时		
			ShutdownCount = 0;				//清零关机计时
			BacklightOn();					//亮度
			EXTI_ClearITPendingBit(EXTI_Line1);		 //清中断标志位
		}
		if(power_temp>2000)
		{
			power_temp=0;
			PowerOffSave();
			BacklightOff();
			TIM_Cmd(TIM5, DISABLE);  //关背光PWM TIM5
			//妈的，上边理论上可以关掉背光，但是就是不能！被逼只能用下边方法了
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
			GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;				
			GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_Init(GPIOA, &GPIO_InitStructure);	    
			GPIO_SetBits(GPIOA,GPIO_Pin_0);//关掉背光
			EXTI_ClearITPendingBit(EXTI_Line1);		 
			GPIO_ResetBits(GPIOC,GPIO_Pin_2);   //power off	
		}
	}
}

void bsp_Power_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_8;				
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;				
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	bsp_delay_ms(600); //只要初始化  口线就会先被拉高  所以要延时  长按开机
	GPIO_Init(GPIOA, &GPIO_InitStructure);	    
	GPIO_SetBits(GPIOA,GPIO_Pin_0);//首先关掉背光
	GPIO_SetBits(GPIOC,GPIO_Pin_2);//开机
	//哔的一声
//	GPIO_SetBits(GPIOC,GPIO_Pin_3);
//	bsp_delay_ms(500);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_3);
	bsp_delay_ms(1);
}
void BacklightOn(void)
{
	TIM_SetCompare1(TIM5,299);//开背光
}
void BacklightOff(void)
{
	TIM_SetCompare1(TIM5,0);//关背光
}
/********************************************************************************************************
  * @函数名称		读取每一个按键接口函数
  * @函数说明   
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*********************************************************************************************************
*/
static uint8_t IsKeyBack(void)  {if ((GPIOC->IDR & GPIO_Pin_6)     == 0) return 1;else return 0;}
static uint8_t IsKeyOk(void) 	{if ((GPIOC->IDR & GPIO_Pin_7)     == 0) return 1;else return 0;}
static uint8_t IsKeyUp(void) 	{if ((GPIOE->IDR & GPIO_Pin_2)     == 0) return 1;else return 0;}
static uint8_t IsKeyDown(void)  {if ((GPIOE->IDR & GPIO_Pin_3)     == 0) return 1;else return 0;}
static uint8_t IsKeyLeft(void)  {if ((GPIOE->IDR & GPIO_Pin_4)     == 0) return 1;else return 0;}
static uint8_t IsKeyRight(void) {if ((GPIOE->IDR & GPIO_Pin_5)     == 0) return 1;else return 0;}

static uint8_t IsKeyDown9(void) {if (IsKeyBack() && IsKeyDown())           return 1;else return 0;}

/*
*********************************************************************************************************
*	函 数 名: bsp_Init_Key
*	功能说明: 初始化按键. 该函数被 bsp_Init() 调用。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Init_Key(void)
{
	InitKeyVar();	/* 初始化按键变量 */
	InitKeyHard();	/* 初始化按键硬件 */
}

//按键初始化函数 
static void InitKeyHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE,ENABLE);
	////////////////////////////////////////////////---key_back---key_ok
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;       //配置为上拉      
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);      
	////////////////////////////////////////////////key_up---key_down---key_left---key_right
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
} 

/*
*********************************************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参:  _KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_tKey.Buf[s_tKey.Write] = _KeyCode;

	if (++s_tKey.Write  >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参:  无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read];

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey2
*	功能说明: 从按键FIFO缓冲区读取一个键值。独立的读指针。
*	形    参:  无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey2(void)
{
	uint8_t ret;

	if (s_tKey.Read2 == s_tKey.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_tKey.Buf[s_tKey.Read2];

		if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
		{
			s_tKey.Read2 = 0;
		}
		return ret;
	}
}
/*
*********************************************************************************************************
*	函 数 名: bsp_GetKeyState
*	功能说明: 读取按键的状态
*	形    参:  _ucKeyID : 按键ID，从0开始
*	返 回 值: 1 表示按下， 0 表示未按下
*********************************************************************************************************
*/
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID)
{
	return s_tBtn[_ucKeyID].State;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetKeyParam
*	功能说明: 设置按键参数
*	形    参：_ucKeyID : 按键ID，从0开始
*			_LongTime : 长按事件时间
*			 _RepeatSpeed : 连发速度
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed)
{
	s_tBtn[_ucKeyID].LongTime = _LongTime;			/* 长按时间 0 表示不检测长按键事件 */
	s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;			/* 按键连发的速度，0表示不支持连发 */
	s_tBtn[_ucKeyID].RepeatCount = 0;						/* 连发计数器 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ClearKey
*	功能说明: 清空按键FIFO缓冲区
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_ClearKey(void)
{
	s_tKey.Read = s_tKey.Write;
}

/*
*********************************************************************************************************
*	函 数 名: InitKeyVar
*	功能说明: 初始化按键变量
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitKeyVar(void)
{
	uint8_t i;

	/* 对按键FIFO读写指针清零 */
	s_tKey.Read = 0;
	s_tKey.Write = 0;
	s_tKey.Read2 = 0;

	/* 给每个按键结构体成员变量赋一组缺省值 */
	for (i = 0; i < KEY_COUNT; i++)
	{
		s_tBtn[i].LongTime = KEY_LONG_TIME;			/* 长按时间 0 表示不检测长按键事件 */
		s_tBtn[i].Count = KEY_FILTER_TIME / 2;		/* 计数器设置为滤波时间的一半 */
		s_tBtn[i].State = 0;							/* 按键缺省状态，0为未按下 */
		//s_tBtn[i].KeyCodeDown = 3 * i + 1;				/* 按键按下的键值代码 */
		//s_tBtn[i].KeyCodeUp   = 3 * i + 2;				/* 按键弹起的键值代码 */
		//s_tBtn[i].KeyCodeLong = 3 * i + 3;				/* 按键被持续按下的键值代码 */
		s_tBtn[i].RepeatSpeed = 0;						/* 按键连发的速度，0表示不支持连发 */
		s_tBtn[i].RepeatCount = 0;						/* 连发计数器 */
	}

	/* 如果需要单独更改某个按键的参数，可以在此单独重新赋值 */
	/* 比如，我们希望按键1按下超过1秒后，自动重发相同键值 */
//	s_tBtn[KID_JOY_U].LongTime = 100;
//	s_tBtn[KID_JOY_U].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

//	s_tBtn[KID_JOY_D].LongTime = 100;
//	s_tBtn[KID_JOY_D].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

//	s_tBtn[KID_JOY_L].LongTime = 100;
//	s_tBtn[KID_JOY_L].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

//	s_tBtn[KID_JOY_R].LongTime = 100;
//	s_tBtn[KID_JOY_R].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */
	/* 判断按键按下的函数 */
	s_tBtn[0].IsKeyDownFunc = IsKeyBack;
	s_tBtn[1].IsKeyDownFunc = IsKeyOk;
	s_tBtn[1].LongTime      = 600;
	s_tBtn[2].IsKeyDownFunc = IsKeyUp;
	s_tBtn[3].IsKeyDownFunc = IsKeyDown;
	s_tBtn[4].IsKeyDownFunc = IsKeyLeft;
	s_tBtn[5].IsKeyDownFunc = IsKeyRight;
//	/* 组合键 */
	s_tBtn[6].IsKeyDownFunc = IsKeyDown9;
	s_tBtn[6].LongTime      = 600;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectKey
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参:  按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectKey(uint8_t i)
{
	KEY_T *pBtn;
	/*
		如果没有初始化按键函数，则报错
		if (s_tBtn[i].IsKeyDownFunc == 0)
		{
			printf("Fault : DetectButton(), s_tBtn[i].IsKeyDownFunc undefine");
		}
	*/
//static KEY_T s_tBtn[KEY_COUNT];
	pBtn = &s_tBtn[i];//读取相应按键的结构体地址，程序里面每个按键都有自己的结构体
	if (pBtn->IsKeyDownFunc())
	{//这个里面执行的是按键按下的处理
/*
**********************************************************************************
下面这个if语句主要是用于按键滤波前,给Count设置一个初值，前面说按键初始化的时候
已经设置了Count = KEY_FILTER_TIME/2
**********************************************************************************
*/		
		if (pBtn->Count < KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
/*
**********************************************************************************
这里实现KEY_FILTER_TIME时间长度的延迟
**********************************************************************************
*/		
		else if(pBtn->Count < 2 * KEY_FILTER_TIME)
		{
			pBtn->Count++;
		}
		else
		{
/*
**********************************************************************************
这个State变量是有其实际意义的，如果按键按下了，这里就将其设置为1，如果没有按下这个
变量的值就会一直是0，这样设置的目的可以有效的防止一种情况的出现：比如按键K1在某个
时刻检测到了按键有按下，那么它就会做进一步的滤波处理，但是在滤波的过程中，这个按键
按下的状态消失了，这个时候就会进入到上面第二步else语句里面，然后再做按键松手检测滤波
，滤波结束后判断这个State变量，如果前面就没有检测到按下，这里就不会记录按键弹起。
**********************************************************************************
*/			if (pBtn->State == 0)
			{
				pBtn->State = 1;

				/* 发送按钮按下的消息 */
				bsp_PutKey((uint8_t)(3 * i + 1));
			}

			if (pBtn->LongTime > 0)
			{
				if (pBtn->LongCount < pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++pBtn->LongCount == pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						bsp_PutKey((uint8_t)(3 * i + 3));
					}
				}
				else
				{
					if (pBtn->RepeatSpeed > 0)
					{
						if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
						{
							pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							bsp_PutKey((uint8_t)(3 * i + 1));
						}
					}
				}
			}
		}
	}//这个里面执行的是按键按下的处理
	else
	{//这个里面执行的是按键松手的处理或者按键没有按下的处理
/*
**********************************************************************************
这里主要实现按键松手的滤波检测
**********************************************************************************
*/		if(pBtn->Count > KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if(pBtn->Count != 0)
		{
			pBtn->Count--;
		}
		else
		{
			if (pBtn->State == 1)
			{
				pBtn->State = 0;

				/* 发送按钮弹起的消息 */
				bsp_PutKey((uint8_t)(3 * i + 2));
			}
		}
		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}//这个里面执行的是按键松手的处理或者按键没有按下的处理
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyScan
*	功能说明: 扫描所有按键。非阻塞，被systick中断周期性的调用
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyScan(void)
{
	uint8_t i;
	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_DetectKey(i);
	}
}

/*
*********************************************************************************************************
* 函 数 名: GUI_KeyTask
* 功能说明: 此函数主要用于得到按键的键值。
* 返 回 值: 无
*********************************************************************************************************
*/
extern void TIM5_PWM_Init(u16 arr,u16 psc);
void GUI_KeyTask(void)
{
	uint8_t ucKeyCode;
	extern uint16_t BacklightTime; 
	extern uint16_t ShutdownCount;              
	extern uint16_t ShutdownFlag;
	ucKeyCode = bsp_GetKey();
	if(ucKeyCode != KEY_NONE)
	{
		BacklightTime = 0;					//清零背光变暗计时
		ShutdownFlag  = 0;					//清零关机计时
		ShutdownCount = 0;
		BacklightOn();
		switch (ucKeyCode)
		{
			case JOY_DOWN_U: /* 上键功能 */
					GUI_StoreKeyMsg(GUI_KEY_UP, 1);			
			break;
			case JOY_LONG_U: /* 上键长按键功能 */
					GUI_SendKeyMsg(GUI_KEY_UP_LONG, 1);
			break;
			case JOY_DOWN_D: /* 下键功能 */
					GUI_StoreKeyMsg(GUI_KEY_DOWN, 1);
			break;
			case JOY_DOWN_L: /* 左键功能 */
					GUI_StoreKeyMsg(GUI_KEY_LEFT, 1);
			break;
			case JOY_DOWN_R: /* 右键功能 */
					GUI_StoreKeyMsg(GUI_KEY_RIGHT, 1);
			break;
			case KEY_DOWN_K2: /* OK键功能 */
					GUI_StoreKeyMsg(GUI_KEY_ENTER, 1);
			break;
			case KEY_LONG_K2: /* OK键长按功能 */
					GUI_StoreKeyMsg(GUI_KEY_ENTER_LONG, 1);
			break;
			case KEY_DOWN_K1: /* ESC DOWN 键功能 */
					GUI_StoreKeyMsg(GUI_KEY_ESCAPE, 1);
			break;
			case KEY_LONG_K1D: /* ESC 长按功能 */
					GUI_StoreKeyMsg(GUI_KEY_ESC_D_LONG, 1);
			break;
			default:/* 其它的键值不处理 */
			break;
		}
	}
}


