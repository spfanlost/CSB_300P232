/**
  ******************************************************************************
  * @file    bsp.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    22-September-2016
  * @brief   This file provides targets hardware configuration 
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

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Initializes the target hardware.
* @param  None
* @retval None
*/
uint32_t BSP_Init (void)
{
	extern uint8_t ModbusDeviceID;

	/* Enable the CRC Module */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//STemWin的库中要使用CRC
	JATG_Disable_SW_Enable();//打开sw两线模式  jtag禁止下载！
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);	//设置NVIC中断分组4:16个抢占优先级，无子优先级
	bsp_delay_init();
	bsp_usart_init(115200);  //串口初始化
	ADC1_Channel_1_init();   //频率电压采集
	ADC2_Channel_2_init();   //声强电压ad配置 
	ADC2_Channel_10_init();  //电池电压ad配置
	bsp_Init_Key();          //薄膜按键初始化
	Read_Stored_Data();      //读取存储的内部变量
	bsp_Relay_Init();        //继电器初始化
	while(RTC_Init());       //RTC初始化,一定要初始化成功
	bsp_Power_init();        //开机初始化
	SaveTabRead();           //读取存储的记录数据
	bsp_OnOffKey_init();     //开机按键初始化	
		TIM4_Configuration();	 //MODBUS专用定时器
		ModInit(ModbusDeviceID);				 //MODBUS设置
	

	return 0;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
