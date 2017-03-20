/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：delay.C
  * 描    述：延时函数文件
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

/*
	全局运行时间，单位1ms
	最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*/
__IO int32_t g_iRunTime = 0;

/********************************************************************************************************
  * @函数名称		延时函数初始化
  * @函数说明   
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*********************************************************************************************************
*/

void bsp_delay_init(void)
{
	/*
		配置systic中断周期为1ms，并启动systick中断。

    	SystemCoreClock 是固件中定义的系统内核时钟，对于STM32F4XX,一般为 168MHz

    	SysTick_Config() 函数的形参表示内核时钟多少个周期后触发一次Systick定时中断.
	    	-- SystemCoreClock / 1000  表示定时频率为 1000Hz， 也就是定时周期为  1ms
	    	-- SystemCoreClock / 500   表示定时频率为 500Hz，  也就是定时周期为  2ms
	    	-- SystemCoreClock / 2000  表示定时频率为 2000Hz， 也就是定时周期为  500us

    	对于常规的应用，我们一般取定时周期1ms。对于低速CPU或者低功耗应用，可以设置定时周期为 10ms
    */
	
	/* Setup SysTick Timer for 1 msec interrupts */
	SysTick_Config(SystemCoreClock / 1000);
}
/*
SysTick计数值最大为24位，最大值为16777215,那么nTime的最大值为233016,
那么void Delay_us(uint32_t nTime)这个函数最大延时时间为233ms,需要更大
的延时时间可以使用void Delay_ms(uint32_t nTime)
*/
void bsp_delay_us(uint32_t n)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;
    
	reload = SysTick->LOAD;                
	ticks  = n * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */  
	
	tcnt   = 0;
	told   = SysTick->VAL;             /* 刚进入时的计数器值 */
    while (1)
    {
        tnow = SysTick->VAL;    
        if (tnow != told)
        {   
            /* SYSTICK是一个递减的计数器 */    
            if (tnow < told)
            {
                tcnt += told - tnow;    
            }
            /* 重新装载递减 */
            else
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            /* 时间超过/等于要延迟的时间,则退出 */
            if (tcnt >= ticks)
            {
            	break;
            }
        }  
    }
//	SysTick->LOAD=72*n;				//装载计数值，因为时钟72M,72次为1us
//	SysTick->CTRL=0x00000005;			//时钟来源为HCLK(72M),打开定时器
//	while(!(SysTick->CTRL&0x00010000));	//等待计数到0
//	SysTick->CTRL=0x00000004;			//关闭定时器
}
void bsp_delay_ms(uint32_t n)
{
	for(;n>0;n--)
	bsp_delay_us(1000);
}
/*
*********************************************************************************************************
*	函 数 名: bsp_GetRunTime
*	功能说明: 获取CPU运行时间，单位1ms。最长可以表示 24.85天，如果你的产品连续运行时间超过这个数，则必须考虑溢出问题
*	形    参:  无
*	返 回 值: CPU运行时间，单位1ms
*********************************************************************************************************
*/
uint32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_INT();  	/* 关中断 */

	runtime = g_iRunTime;	/* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

	ENABLE_INT();  		/* 开中断 */

	return runtime;
}

/*
*********************************************************************************************************
*	函 数 名: SysTick_Handler
*	功能说明: 系统嘀嗒定时器中断服务程序。启动文件中引用了该函数。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void SysTick_Handler(void)
{
	static uint8_t s_count = 0;
	/* 全局运行时间每1ms增1 */
	g_iRunTime++;
	if (g_iRunTime == 0x7FFFFFFF)	/* 这个变量是 int32_t 类型，最大数为 0x7FFFFFFF */
	{
		g_iRunTime = 0;
	}	
	
	if (++s_count >= 10)
	{
		s_count = 0;
		bsp_KeyScan();  /* 每10ms扫描按键一次 */					
	}
}
