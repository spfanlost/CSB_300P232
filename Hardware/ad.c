/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：ad.C
  * 描    述：ad配置文件
  * 创 建 者: 余萌
  * 创建日期: 
  * 修 改 者:             
  * 修改日期: 
  * 版    本: V1.0.0
  ******************************************************************************
  * attention
  *
  ******************************************************************************
  */

#include "bsp.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)

/*******************************************************************************
  * @函数名称	ADC1_Channel_1_init
  * @函数说明  半波测频电压采集通道
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void ADC1_Channel_1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1,ENABLE); 
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 						
	//半波测频输入的IO口初始化PA1_ADC-1_IN1
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;     //ADC1  work mode as independent
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;                  //enable scan mode
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                   //ADC work mode as continuous conversion
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;//sofware set start conversion
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;      //data align as right align
	ADC_InitStructure.ADC_NbrOfChannel       = 1;   					 //1 channel
	ADC_Init(ADC1, &ADC_InitStructure); //init ADC1 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5 ); //ADC1,ADC通道,采样时间为周期	  			    
	ADC_DMACmd(ADC1, ENABLE);   //enable adc1  dma  transmit
	ADC_Cmd(ADC1, ENABLE); 			//enable ADC1	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));  //wait for reset ADC1 calibration falg as reset
	ADC_StartCalibration(ADC1); 	 //start adc1 calibration
	while(ADC_GetCalibrationStatus(ADC1)); 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//软件启动AD转换
}
/*******************************************************************************
  * @函数名称	ADC2_Channel_2_init
  * @函数说明  声强测量
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void ADC2_Channel_2_init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC2, ENABLE );	  //使能ADC2通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	//声强测量输入的IO口初始化PA2_ADC-2_IN2
	GPIO_InitStructure.GPIO_Speed            = GPIO_Speed_50MHz;                        
	GPIO_InitStructure.GPIO_Pin              = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode             = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	ADC_DeInit(ADC2);  //复位ADC,将外设 ADC 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;	      //模数转换工作在多通道模式 挨个转换
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel       = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC2, &ADC_InitStructure);            //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Cmd(ADC2, ENABLE);	//使能ADC2
	ADC_ResetCalibration(ADC2);	//使能复位校准  
	while(ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束
	ADC_StartCalibration(ADC2);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
	ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		                        //使能指定的ADC1的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));                   //等待转换结束
  }
	
/*******************************************************************************
  * @函数名称	ADC2_Channel_3_init
  * @函数说明   全波测频（暂时没用）
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void ADC2_Channel_3_init(void)
{
	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC2	, ENABLE );	  //使能ADC1通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	//全波测频输入的IO口初始化PA3_ADC-2_IN3
	GPIO_InitStructure.GPIO_Speed            = GPIO_Speed_50MHz;                        
	GPIO_InitStructure.GPIO_Pin              = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode             = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	ADC_DeInit(ADC2);  //复位ADC1,将外设 ADC2 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;	      //模数转换工作在多通道模式 挨个转换
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel       = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC2, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Cmd(ADC2, ENABLE);	//使能ADC2
	ADC_ResetCalibration(ADC2);	//使能复位校准  
	while(ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束
	ADC_StartCalibration(ADC2);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
}

	
/*******************************************************************************
  * @函数名称	ADC2_Channel_10_init
  * @函数说明  电测电压测量
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void ADC2_Channel_10_init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC2	, ENABLE );	  //使能ADC1通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	//电池电压测量输入的IO口初始化PC0_ADC-2_IN10
	GPIO_InitStructure.GPIO_Speed            = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin              = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode             = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	ADC_DeInit(ADC2);  //复位ADC1,将外设 ADC2 的全部寄存器重设为缺省值
	ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode       = DISABLE;	      //模数转换工作在多通道模式 挨个转换
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel       = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC2, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   
	ADC_Cmd(ADC2, ENABLE);	//使能ADC2
	ADC_ResetCalibration(ADC2);	//使能复位校准  
	while(ADC_GetResetCalibrationStatus(ADC2));	//等待复位校准结束
	ADC_StartCalibration(ADC2);	 //开启AD校准
	while(ADC_GetCalibrationStatus(ADC2));	 //等待校准结束
  }

/*******************************************************************************
  * @函数名称	ADC2_Channel_2_get
  * @函数说明  声强测量 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
uint16_t ADC2_Channel_2_get(void)//声强测量
{
    //设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5 );	//ADC,ADC通道,采样时间为239.5周期	  			    
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		                        //使能指定的ADC1的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));                   //等待转换结束
	return ADC_GetConversionValue(ADC2);	                            //返回最近一次ADC1规则组的转换结果
}

/*******************************************************************************
  * @函数名称	ADC1_Channel_1_get
  * @函数说明   全波测频
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
//uint16_t ADC2_Channel_3_get(void)//全波测频
//{
//    //设置指定ADC的规则组通道，一个序列，采样时间
//	ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
//	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		                        //使能指定的ADC1的软件转换启动功能	
//	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));                   //等待转换结束
//	return ADC_GetConversionValue(ADC2);	                            //返回最近一次ADC1规则组的转换结果
//}

/*******************************************************************************
  * @函数名称	ADC2_Channel_10_get
  * @函数说明  电测电压测量  
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
uint16_t ADC2_Channel_10_get(void)//电测电压测量
{
    //设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);		                        //使能指定的ADC1的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));                     //等待转换结束
	return ADC_GetConversionValue(ADC2);	                            //返回最近一次ADC1规则组的转换结果
}

/*******************************************************************************
  * @函数名称	Get_Bat_Average
  * @函数说明  电池求均值 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
uint16_t Get_Bat_Average(void)
{
//中位值平均滤波法（又称防脉冲干扰平均滤波法）
	const int FILTER_N = 100;

	int i, j;
	uint32_t filter_temp, filter_sum = 0;
	int filter_buf[FILTER_N];
	for(i = 0; i < FILTER_N; i++) {
	filter_buf[i] = ADC2_Channel_10_get();
	}
	// 采样值从小到大排列（冒泡法）
	for(j = 0; j < FILTER_N - 1; j++) {
		for(i = 0; i < FILTER_N - 1 - j; i++) {
		  if(filter_buf[i] > filter_buf[i + 1]) {
			filter_temp = filter_buf[i];
			filter_buf[i] = filter_buf[i + 1];
			filter_buf[i + 1] = filter_temp;
		  }
		}
	}
	// 去除最大最小极值后求平均
	for(i = 10; i < FILTER_N - 10; i++) filter_sum += filter_buf[i];
	return filter_sum / (FILTER_N - 20);
} 	 




