/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：pwm.c
  * 描    述：文件
  * 创 建 者: 余萌
  * 创建日期: 
  * 修 改 者:             
  * 修改日期: 
  * 版    本: V1.0.0
  ******************************************************************************
  * attention
  *	定时器API
  ******************************************************************************
*/

#include "bsp.h"
//液晶背光控制初始化
void TIM5_PWM_Init(uint16_t arr,uint16_t psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	//使能定时器3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	//设置该引脚为复用输出功能,输出TIM5 CH1的PWM脉冲波形	GPIOA.0
	GPIO_InitStructure.GPIO_Pin             = GPIO_Pin_0; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode            = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed           = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
	//初始化TIM5
	TIM_TimeBaseStructure.TIM_Period        = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler     = psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	//初始化 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode          = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity      = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM5, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM OC1
	TIM_CtrlPWMOutputs(TIM5,ENABLE);	//MOE 主输出使能	
	TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);  //CH1预装载使能
	TIM_ARRPreloadConfig(TIM5, ENABLE); //使能TIMx在ARR上的预装载寄存器
	TIM_Cmd(TIM5, ENABLE);  //使能背光PWM TIM5
}

////通用定时器中断初始化
////这里时钟选择为APB1的2倍，而APB1为36M
////arr：自动重装值。
////psc：时钟预分频数
//void TIM2_Int_Init(uint16_t number,uint16_t psc)
//{
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
//	TIM_DeInit(TIM2);
//	TIM_TimeBaseStructure.TIM_Period                     = number; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 
//	TIM_TimeBaseStructure.TIM_Prescaler                  = psc; //设置用来作为TIMx时钟频率除数的预分频值  
//	TIM_TimeBaseStructure.TIM_ClockDivision              = 0; //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode                = TIM_CounterMode_Up;  //TIM向上计数模式
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
//	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
//	NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;            //TIM2中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;					 //先占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;         			 //从优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;               //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);
//	TIM_Cmd(TIM2, ENABLE);  //使能TIM	
//}
////测试定时器每ms中断一次
//void TIM2_IRQHandler(void)   //TIM3中断
//{	
//	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
//	{
//		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIMx更新中断标志 
//	}
//}

