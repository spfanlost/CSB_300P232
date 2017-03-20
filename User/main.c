/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    22-September-2016
  * @brief   This file provides main program functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// 软件版本定义
//#define CSB300P232
//#define CSB300P485
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* 本地变量定义 --------------------------------------------------------------*/ 
uint16_t     BacklightTime = 0; 
uint16_t     UpdateBatTime = 0;
uint16_t     ShutdownFlag  = 0;
uint16_t     ShutdownCount = 0;

uint16_t     SaveGearsValue = 1;     //档位
uint16_t	 PowrOffTime    = 10;

/* 外部变量声明 ---------------------------------------------------------------*/ 
//#define REG_INPUT_START 1
//#define REG_INPUT_NREGS 20
//#define REG_HOLDING_START 	1
//#define REG_HOLDING_NREGS 	10
///* ----------------------- Static variables ---------------------------------*/
//static USHORT   usRegInputStart = REG_INPUT_START;
//static USHORT   usRegInputBuf[REG_INPUT_NREGS];
//static USHORT   usRegHoldingStart = REG_HOLDING_START;
//static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */ 
int main(void)
{
	/*** 函数内部变量定义 ***/
	uint16_t main_temp = 0;
	/*** 函数外部变量声明 ***/
	extern volatile float frequency;
	extern volatile float ultrasonic_sound;
	extern uint8_t AutoGearFlag ; //默认是0
	
	/* Initilize the BSP layer */
	BSP_Init();
	TFT_Init();
	GUI_Init();
	CreatPageStartWin();
	TIM5_PWM_Init(299,0);//背光PWM
	BacklightOn(); 
	MainTask();
	if(WM_IsWindow(WinPara.hWinStatus))//判断窗口是否有效
	{
		WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_BATTERY); /* 更新电池电量发送消息 */				
		WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_GAUTO);		
	}
	IWDG_Init(); //初始化看门狗6.553s喂狗
	while(1)
	{
		IWDG_Feed();//6.553s内喂狗
		update_ultrasonic();
		update_frequence();
		if(ultrasonic_sound<0.05f)  //长时间声强为零的处理
		{
			frequency = 0;
			ultrasonic_sound=0;
			main_temp++;		//只要有一次为零就开始计是不是一直为零
			if(main_temp >= 5)  //声强一直为零，才开始进入关机处理
			{
				main_temp = 5;
				if(PowrOffTime != 0)             //开机后声强测量长时间为零 开始关机倒计时
				{
					ShutdownFlag = 1;			  //开始关机计时
					if(ShutdownCount >= PowrOffTime*60) //时间到
					{
						ShutdownFlag = 0;
						ShutdownCount = 0;
						PowerOffSave();
						BacklightOff();
						TIM_Cmd(TIM5, DISABLE);  //关背光PWM TIM5
						GPIO_ResetBits(GPIOC,GPIO_Pin_2);   //power off		
					}
				}
			}
		}
		else
		{
			main_temp = 0;	//声强不一直为零，就不能关机
			ShutdownFlag = 0; //只要声强不为零就一直不能关机
			ShutdownCount = 0;
			if(AutoGearFlag == 1) //刷新档位标志
			{
				RefreshGear();
			}
		}
		if(WM_IsWindow(WinPara.hWinOne)) //判断窗口是否有效
		{
			WM_SendMessageNoPara(WinPara.hWinOne,MY_MSG_FREQUENCY); /*	发送消息 */			
			WM_SendMessageNoPara(WinPara.hWinOne,MY_MSG_ULTRASONIC); /*	发送消息 */			
		}
		if(WM_IsWindow(WinPara.hWinTwo)) //判断窗口是否有效
		{
			WM_SendMessageNoPara(WinPara.hWinTwo,MY_MSG_ULTRASONIC); /*	发送消息 */			
			WM_SendMessageNoPara(WinPara.hWinTwo,MY_MSG_ADD_GDATA); /*	发送消息 */			
		}
		if( UpdateBatTime >= 30 )// 30s更新电池电量
		{
			UpdateBatTime=0;
			if(WM_IsWindow(WinPara.hWinStatus)) 									//判断窗口是否有效
			{
				WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_BATTERY); /* 更新电池电量发送消息 */				
			}
		}
		if( BacklightTime >= 20 )//	20s背光变暗	
		{
			BacklightTime = 20;
			TIM_SetCompare1(TIM5,10);//设背光亮度：0最暗，299最亮
		}

//	NumFreeBytes = GUI_ALLOC_GetNumFreeBytes();		
//	printf("GUIFreeBytes = %ldK\r\n", NumFreeBytes/1024);
	GUI_Delay(100);		
	}
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
