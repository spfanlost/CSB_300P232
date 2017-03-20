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
  * 超声处理相关API
  ******************************************************************************
*/

#include "bsp.h"
#include "stm32_dsp.h"
#include <math.h>

#define NPT 1024              //FFT点数

/* 本地变量定义 ---------------------------------------------------------*/ 
volatile float   ultrasonic_sound        = 0.00f;//全局变量
volatile float   ultrasonic_sound_max    = 0.00f;//全局变量
volatile float   frequency               = 0.00f;//全局变量

static long       lBUFMAG[NPT+NPT/2];			   //存储求模后的数据
static long       lBUFOUT[NPT];                    //FFT输出序列
static long       lBUFIN[NPT];                     //FFT输入系列

static uint16_t   frequency_ad[1024] = {0};
static uint16_t   ultrasonic_ad[100] = {0};

static float ultrasonic_last[4] = {0.0f};
static float Ultra_PowerTemp = 0.0f;

float	          GVotage            = 0.0f;		//测量声强电压用来测算档位
uint16_t     	  ConectFrqcyOnOff   = 1;           //声强计算时候是否关联频率 默认打开 //全局变量
uint16_t          RangeMAX = 2540;                   //RangeMAX严禁为零！！！//全局变量
uint16_t          k_value = 100;                    //  //全局变量

/* 外部变量声明 ---------------------------------------------------------*/ 
extern uint16_t GearsValue;

//这里的传输形式是固定的,这点要根据不同的情况来修改
//从外设模式->存储器/16位数据宽度/存储器增量模式
/**
 * [myDMA_Config DMA1的各通道配置]
 * @param DMA_CHx     [DMA通道CHx]
 * @param peri_add    [外设地址]
 * @param memery_add  [存储器地址]
 * @param data_number [数据传输量]
 */
void myDMA_Config(DMA_Channel_TypeDef* DMA_CHx,uint32_t peri_add,uint32_t memery_add,uint16_t data_number)
{	  
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = peri_add;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr     = memery_add;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设发送到内存  DMA_CCRX位4
	DMA_InitStructure.DMA_BufferSize         = data_number;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord; //内存数据宽度为16位
	DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;  //工作在循环缓存模式
	DMA_InitStructure.DMA_Priority           = DMA_Priority_High; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure); 
	DMA_Cmd(DMA1_Channel1, DISABLE);        //启动DMA通道  	
} 
//开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx, uint16_t DMA1_MEM_LEN)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭ADC1 DMA1 所指示的通道      
	DMA_SetCurrDataCounter(DMA_CHx,DMA1_MEM_LEN);//DMA通道的DMA缓存的大小
	DMA_Cmd(DMA_CHx, ENABLE);  //使能ADC1 DMA1 所指示的通道 
}

/*******************************************************************************
* 函数名称: quicksort()
* 功能描述: 快排算法
* 输入参数: left，right，要排序数组的左右端点 如a[n]，则quicksort(0,n-1); 
* 返回参数: 无
********************************************************************************/
static void quicksort(int left,int right) 
{
    int i,j;
	uint32_t t,temp; 
    if(left>right) 
       return; 	
    temp=ultrasonic_ad[left]; //temp中存的就是基准数 
    i=left; 
    j=right; 
    while(i!=j) 
    { 
       //顺序很重要，要先从右边开始找 
       while(ultrasonic_ad[j]>=temp && i<j) 
                j--; 
       //再找右边的 
       while(ultrasonic_ad[i]<=temp && i<j) 
                i++; 
       //交换两个数在数组中的位置 
       if(i<j) 
       { 
                t=ultrasonic_ad[i]; 
                ultrasonic_ad[i]=ultrasonic_ad[j]; 
                ultrasonic_ad[j]=t; 
       } 
    } 
    //最终将基准数归位 
    ultrasonic_ad[left]=ultrasonic_ad[i]; 
    ultrasonic_ad[i]=temp; 
                             
    quicksort(left,i-1);//继续处理左边的，这里是一个递归的过程 
    quicksort(i+1,right);//继续处理右边的 ，这里是一个递归的过程 
} 


/*******************************************************************************
* 函数名称: read_ultrasonic_once()
* 功能描述: 读取一次声强数据
* 输入参数: 
* 返回参数: 无
********************************************************************************/
/**
***		档位1实验所得数据
***		
***		外部输入电压		0 ---------- 5 ------------ 25 ---------- >50
***		
***		AD输入电压		0 ---------- 0 ---------- 1.187 --------- 3
***		
***		显示值范围		0 --------------------------------------- RangeMAX/100
***		
***						 
***		档位2实验所得数据
***		
***		外部输入电压		0 ---------- 5 ---------- 25
***		
***		AD输入电压		0 --------- 0.67 ------- 2.93
***		
***		显示值范围		0 ---------------------- RangeMAX*0.5/100
***		
***						 
***		档位3实验所得数据
***		
***		外部输入电压		0 ---------- 5
***		
***		AD输入电压		0 --------- 1.67		
***		
***		显示值范围		0 --------- RangeMAX*0.1/100
***		
**/

//void ult_delay_us(uint32_t us)
//{
//	us = us*72;
//	while(--us){}
//}
//ult_delay_us(10);
/*********更新声强值 ultrasonic_sound************/
void update_ultrasonic(void)
{
	uint16_t i=0;
	uint16_t temp1=0;
	uint32_t Sum1=0;
	for(i=0;i<100;i++)
	{
		ultrasonic_ad[i] = ADC2_Channel_2_get();
	}
	quicksort(0,99);//快排算法   
	for(i=15;i<85;i++)
	{
		Sum1 += ultrasonic_ad[i];
	}
//	Ultra_PowerTemp=(float)(Sum1/70)*(3.285/4096);
	Ultra_PowerTemp=(float)(Sum1/70)*(0.000802f);
	if(Ultra_PowerTemp <= 0.035f) Ultra_PowerTemp=0;  //电压阀值
	if(Ultra_PowerTemp >= 3.285f) Ultra_PowerTemp=3.285f;  //电压阀值
	GVotage = Ultra_PowerTemp;			//用来测算档位
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	switch (GearsValue)
    {
    	case 1:
//			Ultra_PowerTemp=Ultra_PowerTemp*Ultra_PowerTemp*RangeMAX/（100*3*3*4）;	
		
//			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX/（100*3*4）;
			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX*0.000833f;
    		break;
    	case 2:
//			Ultra_PowerTemp=Ultra_PowerTemp*Ultra_PowerTemp*RangeMAX/（100*2*2.93*2.93*4）;	
		
//			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX/（100*2*2.93*4）;
			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX*0.000427f;
    		break;
    	case 3:
//			Ultra_PowerTemp=Ultra_PowerTemp*Ultra_PowerTemp*RangeMAX/（100*10*1.67*1.67*4）;	
		
//			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX/（100*10*1.67*4）;
			Ultra_PowerTemp=Ultra_PowerTemp*RangeMAX*0.00015f;
    		break;
    	default:
    		break;
    }	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(ConectFrqcyOnOff)              //计算时候关联频率
	{
		Ultra_PowerTemp=Ultra_PowerTemp*(1+frequency*0.075f);
	}
	else
	{
		Ultra_PowerTemp=Ultra_PowerTemp*4;	
	}

	for(i=0;i<3;i++)                    //压栈
	{
		ultrasonic_last[3-i]=ultrasonic_last[2-i];
	}
		ultrasonic_last[0] = Ultra_PowerTemp;

	/*探棒拿出 立刻清零声强值*/
	if(ultrasonic_last[0]<0.05f)
	{
		Ultra_PowerTemp=0;
		/*探棒拿出后立刻清楚缓存区 防止再次放入时候会出现声强值缓慢上升的情况*/
		for(i=0;i<4;i++)
		{
			ultrasonic_last[i]=0;
		}
	}
	else
	{
		Ultra_PowerTemp=ultrasonic_last[0]*0.5f+ultrasonic_last[1]*0.2f+ultrasonic_last[2]*0.2f+ultrasonic_last[3]*0.1f;					
	}

	Ultra_PowerTemp=Ultra_PowerTemp*k_value*0.01f;
	
	temp1=(uint16_t)(Ultra_PowerTemp*100);
	Ultra_PowerTemp=(float)(temp1*0.01f);     //保留小数点后两位 后面舍弃

	switch (GearsValue)
    {
    	case 1:
			if(Ultra_PowerTemp > (RangeMAX*0.01f))   //RangeMAX/100
			{
				Ultra_PowerTemp = RangeMAX*0.01f;			
				ultrasonic_sound_max = RangeMAX*0.01f;
			}
			else if(Ultra_PowerTemp<0.05f)  
			{
				Ultra_PowerTemp=0;	
				ultrasonic_sound_max=0;
			}
    		break;
    	case 2:
			if(Ultra_PowerTemp > RangeMAX*0.005f)   //RangeMAX/100/2
			{
				Ultra_PowerTemp = RangeMAX*0.005f;
				ultrasonic_sound_max = RangeMAX*0.005f;
			}
			else if(Ultra_PowerTemp<0.05f)  
			{
				Ultra_PowerTemp=0;	
				ultrasonic_sound_max=0;
			}
    		break;
    	case 3:
			if(Ultra_PowerTemp > RangeMAX*0.001f)   //RangeMAX/100/10
			{
				Ultra_PowerTemp = RangeMAX*0.001f;
				ultrasonic_sound_max = RangeMAX*0.001f;			
			}
			else if(Ultra_PowerTemp<0.05f)  
			{
				Ultra_PowerTemp=0;	
				ultrasonic_sound_max=0;
			}
    		break;
    	default:
    		break;
    }
	if(Ultra_PowerTemp>ultrasonic_sound_max)
	{
		ultrasonic_sound_max = Ultra_PowerTemp;
	}
	ultrasonic_sound = Ultra_PowerTemp;
}

//fft求幅值
static void powerMag(long nfill)
{
	int32_t lX,lY;
	uint32_t i;
	for (i=0; i < nfill; i++) 
	{
		lX= (lBUFOUT[i]<<16)>>16; /* sine_cosine --> cos */ 
		lY= (lBUFOUT[i] >> 16);   /* sine_cosine --> sin */     
		{
			float X=  NPT*((float)lX)/32768; 
			float Y = NPT*((float)lY)/32768; 
			float Mag = sqrt(X*X+ Y*Y)/nfill;  // 先平方和,再开方 
			lBUFMAG[i] = (long)(Mag*65536); 
		}
	} 
}

/* 
********************************************************************************************************* 
说明:(快速傅里叶变换)获取频率
********************************************************************************************************* 
*/ 
void update_frequence(void)
{
	uint16_t    i=0;
	uint16_t    j=0;
	uint32_t    temp=0;
	long    	temp1=0;
	float 		FrequencyTemp=0.0f;
	myDMA_Config(DMA1_Channel1,(uint32_t)&ADC1->DR,(uint32_t)&frequency_ad,NPT);   //每次重新配置  让内存地址指针偏移到首地址		
	MYDMA_Enable(DMA1_Channel1,NPT); //开启一次DMA传输
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	while(1)
	{
		if(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=RESET)	//判断通道1传输完成
		{
			DMA_ClearFlag(DMA1_FLAG_TC1);//清除通道1传输完成标志
			break; 
		}				
	}			    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(i=0;i<NPT;i++)
	{
		lBUFIN[i]=frequency_ad[i]<<16;					 
	}
	cr4_fft_1024_stm32(lBUFOUT,lBUFIN,NPT);
	powerMag(NPT);                      
	temp1=0;				
	for(i=10;i<500;i++)                    //从20开始 因为有时会检测到非超声波频段频率  这里屏蔽了
	{
		if(lBUFMAG[i]>temp1)
		{
			temp1=lBUFMAG[i];
			j=i;
		}
	}		
//	FrequencyTemp=((float)(j*0.65149f)-0.02f);	   //计算频率56M,ADC_SampleTime_1Cycles5
	FrequencyTemp=(float)(j*0.586f);	           //计算频率72M,ADC_SampleTime_7Cycles5
	temp=(uint32_t)(FrequencyTemp*100);
	FrequencyTemp=(float)(temp*0.01f);    				 	//保留小数点后两位 后面舍弃
	if(FrequencyTemp>290.0f)                         //量程限制
	{
		FrequencyTemp=290.0f;                 
	}	
	if(FrequencyTemp<12.0f)                         //量程限制
	{
		FrequencyTemp=0;                 
	}
	frequency=FrequencyTemp;
}


/******************end file*********************/





