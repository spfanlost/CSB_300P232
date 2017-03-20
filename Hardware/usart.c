
/*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：usart.c
  * 描    述：文件
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

uint8_t	idx = 0;
FlagStatus RXStart;

uint16_t ReadTimeTabFlag = 0;
uint16_t ReadDataTabFlag = 0;

uint16_t ContTimeNum = 0;
uint16_t ContDataNum = 0;
uint16_t TimeN = 0;

uint16_t HoldReg[100] = {0};
uint8_t HaveMes,Tim_Out,Rcv_Complete,Comu_Busy,Rcv_Num,Send_Num;
uint8_t Rcv_Data,Send_Data;
uint8_t *PointToRcvBuf,*PointToSendBuf;
uint8_t Rcv_Buffer[110],Send_Buffer[110];	//Rcv_Buffer中第一个元素放收到数目，第二元素开始才是modbus协议内容
uint8_t ModbusDeviceID=1;

extern volatile float frequency;
extern volatile float ultrasonic_sound;
extern uint16_t k_value;				//默认K值---0（0-1000）
extern uint16_t SaveGearsValue ;        //默认档位---1（1，2，3，4）
extern uint16_t GearsValue ;
extern uint8_t  AutoGearFlag ;
extern uint16_t PowrOffTime ;           //默认关机时间---2{0（关闭），1（10min），2（20min），3（30min）}
extern uint16_t LogoOnOff ;             //
extern uint16_t SaveTimeCnt ;
extern uint16_t ConectFrqcyOnOff;          //声强计算时候是否关联频率   默认打开
extern uint16_t RangeMAX; //RangeMAX严禁为零！！！
extern uint16_t SaveTimeJG;
static void Delay(volatile unsigned int nCount);

//////////////////////////////////////////////////////////////////
 /*
 * 函数名：TIM4_Configuration()
 * 描述  ：MODBUS专用定时器，TIM4初始化 
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
	/*
	3.5个字符时间区分不同的帧，即接收到的两个字符之间时间间隔小于3.5个字符
	时间时认为是同一个帧的，如果间隔大于3.5个字符时间则认为是不同帧的
	在一般的串口通信中，发送1个字符需要：1位起始位，8位数据位，1位校验位(可无),
	1位停止位,总共 1+8+1+1 = 11位，3.5个字符就是 3.5 * 11 = 38.5位，
	假如波特率是115200,那么传输1位的时间是1000/115200 (ms) ,
	这样，3.5个字符时间就大约是 (1000/115200)*38.5(ms)=0.334ms ,即定时器需要的中断时间
	若波特率是9600，3.5个字符时间就大约是 (1000/9600)*38.5(ms)=4.01ms 
	*/
  //预分频系数=7200-1，那么7200/72M = 0.0001,即每100us计数值加1
  //若波特率是115200 自动重装载值=10-1，那么100us x 10 = 1ms,即1ms中断一次	
  //若波特率是9600 自动重装载值=50-1，那么100us x 50 = 5ms,即5ms中断一次	
void TIM4_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructure.TIM_Period        = 50-1;              /* 自动重装载寄存器周期的值(计数值) */
	/* 累计 TIM_Period个频率后产生一个更新或者中断 */
	TIM_TimeBaseStructure.TIM_Prescaler     = 7200 - 1;           /* 时钟预分频数 72M/72 */
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        /* 采样分频 */
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  /* 向上计数模式 */
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);                         /* 清除溢出中断标志 */
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel                   = TIM4_IRQn;            //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;                    //先占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;                    //从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;               //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);                                        /* 开启定时器 */  
}
//#define	RS485_TX_EN() GPIO_SetBits(GPIOD,GPIO_Pin_6)
//#define	RS485_RX_EN() GPIO_ResetBits(GPIOD,GPIO_Pin_6)
void bsp_usart_init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//使能USART1，GPIOA,GPIOD时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD, ENABLE);	

	USART_DeInit(USART1);  //复位串口1
	
	GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed                 = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_Out_PP;//RX485输出
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2 ;                   //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;                    //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;               //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);                                             
	
	USART_InitStructure.USART_BaudRate            = bound;                           //一般设置为9600;
	USART_InitStructure.USART_WordLength          = USART_WordLength_9b;             //字长为8位数据格式
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;                //一个停止位
	USART_InitStructure.USART_Parity              = USART_Parity_Even;               //奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件数据流控制
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;	 //收发模式
	USART_Init(USART1, &USART_InitStructure);                                        //初始化串口
	USART_Cmd(USART1, ENABLE);
	
	while((USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET));
	Delay(50);
//	RS485_RX_EN();
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                                   //开启接收中断
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */

}

//CRC校验查表用参数
/* CRC 高位字节值表*/
static uint8_t auchCRCHi[] = {
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
0x80,0x41,0x00,0xC1,0x81,0x40
} ;
/* CRC低位字节值表*/
static uint8_t auchCRCLo[] = {
0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
0x43,0x83,0x41,0x81,0x80,0x40
} ;

  /* 函数功能：CRC校验用函数
 	函数输入：puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数														    
	函数输出：计算出来的CRC校验码。                                                                                      */
uint16_t CRC16(uint8_t *puchMsgg,uint8_t usDataLen)//puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数 
{
    uint8_t uchCRCHi = 0xFF ; /* 高CRC字节初始化*/
    uint8_t uchCRCLo = 0xFF ; /* 低CRC 字节初始化*/
    uint8_t uIndex ; /* CRC循环中的索引*/
    while (usDataLen--) /* 传输消息缓冲区*/
    {
    uIndex = uchCRCHi ^ *puchMsgg++ ; /* 计算CRC */
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
    uchCRCLo = auchCRCLo[uIndex] ;
    }
    return ((uchCRCHi << 8) | uchCRCLo) ;
}

/*	  
 * 函数功能：用于Modbus初始化
 * 函数输入：Id为Modbus站号。														    
 * 函数输出：无。 
 */

void ModInit(uint8_t Id)
{
	//modbus参数初始化
	PointToRcvBuf=Rcv_Buffer;
	PointToSendBuf=Send_Buffer;
	Send_Num=1;//发送的数据顺序（输出数组的第几个数）
	ModbusDeviceID=Id;//站号设置
	Rcv_Buffer[1]=ModbusDeviceID;
	Send_Buffer[1]=ModbusDeviceID;
	Comu_Busy=0;
	HaveMes=0;
	Rcv_Complete=1;
}

/* 函数功能：错误帧处理（处理1,2,3,6四类错误，其中1为不合法功能码，2不合法数据地址，3不合法数据，6从机设备忙碌）
函数输入：第一个参数Mode用来指示哪一类错误，
pointer用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
后面的元素按照Modbus协议组织。+
函数输出：无。
*/
void  ErrorHandle(uint8_t Mode,uint8_t *Pointer)
{
		uint16_t SendKey;//要发送数据的校验值
		HaveMes=0;//清除信息位
		TIM_Cmd(TIM4,DISABLE);
		TIM_SetCounter(TIM4,0);
		Rcv_Complete=1;
		Comu_Busy=1;
		Rcv_Buffer[0]=Rcv_Num;
		switch(Mode)
	  {
			case 1:*(Pointer+3)=0x01;//错误功能码
			break;			 
			case 2:*(Pointer+3)=0x02;//错误地址
			break;
			case 3:*(Pointer+3)=0x03;//错误数据
			break;
			case 6:*(Pointer+3)=0x06;//从设备忙
			break;
	  }
	  *Pointer=0x05;//输出寄存器有效数据个数	
	  *(Pointer+2)|=0x80;//功能码最高位置一
	  //写入校验码
	  SendKey=CRC16(Pointer+1,*Pointer-2);					
	  //将计算出来的校验码装入输出数据缓存中
	  *(Pointer+(*Pointer-1))=(uint8_t)(SendKey>>8);
	  *(Pointer+(*Pointer))=(uint8_t)(SendKey&0x00FF);

//		RS485_TX_EN();	
		/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
		Delay(50);
		/*	USART_IT_TXE ENABLE	*/ 
		USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
		/*	USART_IT_RXNE DISABLE	*/ 
		USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 															   
}
/* 函数功能：用于Modbus信息接收
函数输入：无。														    
函数输出：无*/
void ModRcv(void)
{
	HaveMes=1;//表示接收到了信息
	Rcv_Data=USART_ReceiveData(USART1);
	if(Comu_Busy!=1)//如果不忙，可以接收下一帧信息
	{
		TIM_Cmd(TIM4, DISABLE);
		TIM_SetCounter(TIM4,0);
		//如果间隔时间超过了3.5个字符，同时接受的字节和自己的站号一致，则认为接收开始
		if((Tim_Out!=0)&&(Rcv_Data==ModbusDeviceID))
		{
			Rcv_Buffer[1]=ModbusDeviceID;
			Send_Buffer[1]=ModbusDeviceID;
			Rcv_Complete=0;//表示数据帧接收开始
			Rcv_Num=0;//接收数据个数初始化
			Rcv_Num++;//同时个数加一
		}
		else if((Tim_Out!=0)&&(Rcv_Data==0))//广播地址
		{
			Rcv_Buffer[1]=0;
			Send_Buffer[1]=ModbusDeviceID;
			Rcv_Complete=0;//表示数据帧接收开始
			Rcv_Num=0;//接收数据个数初始化
			Rcv_Num++;//同时个数加一
		}

		if((0==Tim_Out)&&(0==Rcv_Complete))//如果处于接收一帧的正常过程中
		{
			if(Rcv_Num<100)
			{
				Rcv_Buffer[Rcv_Num+1]=Rcv_Data;//将数据放入接收数组中
				Rcv_Num++;//同时个数加一	
			}
			else
			{
				Rcv_Complete=1;
				Comu_Busy=1;
				Rcv_Buffer[0]=Rcv_Num;
				*(PointToSendBuf+2)=*(PointToRcvBuf+2);//获取功能码
				ErrorHandle(6,PointToSendBuf);//表示超出了字节数(从机设备忙碌)
				Rcv_Num=0;
			}
		}
		Tim_Out=0;
		TIM_Cmd(TIM4, ENABLE);
	}
}
/* 函数功能：用于Modbus信息发送
 * 函数输入：无。														    
 * 函数输出：无。
 */																
void ModSend(void)
{
 	Send_Data=*(PointToSendBuf+Send_Num);
	USART_SendData(USART1,Send_Data);
	Send_Num++;
	if(Send_Num>(*PointToSendBuf))//发送已经完成
	{
		Comu_Busy=0;
		*PointToSendBuf=0;
		Rcv_Num=0;
		Send_Num=1;

		/*RS485由发送切换至接收时，务必等待数据发送完成后，再切换。否则造成数据丢失*/
		while((USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET));
//		RS485_RX_EN();	
		/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
		Delay(50);
		/*	USART_IT_TXE ENABLE	*/ 
		USART_ITConfig(USART1,USART_IT_TXE,DISABLE);
		/*	USART_IT_RXNE ENABLE	*/ 
		USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); 
	}
}

 /* 函数功能：读取从机ID
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。
	                                                                                      */
void ReadDeviceID(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
{
	uint16_t SendKey;//要发送数据的校验值
	if(*(pointer_1)==4)	  //如果接收到的字节数不是4个，就是一个错误帧
	{
		if(*(pointer_1+1)==00)	//是否是广播帧
		{
			*(pointer_2)=0x06;//有效字节个数
			*(pointer_2+1)=0x00;//广播帧头
			*(pointer_2+2)=0x02;//响应02个字节
			*(pointer_2+3)=ModbusDeviceID;//ModbusDeviceID //232版本的默认ID是1
			*(pointer_2+4)=0x00;//保留
			//写入校验码						   
			SendKey=CRC16(pointer_2+1,*pointer_2-2);					
			//将计算出来的校验码装入输出数据缓存中
			*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
			*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);

//			RS485_TX_EN();	
			/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
			Delay(50);
			/*	USART_IT_TXE ENABLE	*/ 
			USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
			/*	USART_IT_RXNE DISABLE	*/ 
			USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 
		}
		else
		{
			ErrorHandle(2,pointer_2);//错误起始地址
		}
	}
	else
	{
		Comu_Busy=0;
	}
}




 /* 函数功能：读取保持寄存器
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。
 */
void ReadHoldingReg(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
{
	uint16_t Address=0;//待读取寄存器起始地址（HoldReg[i],i为0-99对应地址从0到99）
	uint16_t Num=0;//要读取的寄存器个数
	uint16_t SendKey;//要发送数据的校验值
	Address=(uint16_t)(*(pointer_1+3))*256+(*(pointer_1+4));//先得到寄存器起始地址
	Num=(uint16_t)(*(pointer_1+5))*256+(*(pointer_1+6));//先得到要读取的寄存器个数
	*(pointer_2+2)=0x03;//第三个字节为功能码
	if(*(pointer_1)==8)	  //如果接收到的字节数不是8个，就是一个错误帧
	{
		if(Address<=100) //只要地址小于100，就是合法地址
		{
			if(Address+Num<=100&&Num>0) //只要地址加数量大于0小于100，就是合法数量
			{
				
 				//用于for循环
				uint8_t i;
				uint8_t j;

				*(pointer_2+3)=Num*2;//第四个字节为要发送的字节个数
				*(pointer_2)=1+1+1+Num*2+2;//有效字节个数等于丛机地址+功能码+字节个数+寄存器信息+CRC校验
	
	
				for(i=Address,j=4;i<Address+Num;i++,j+=2)
				{
					*(pointer_2+j)=(uint8_t)(HoldReg[i]>>8);//先放高位
					*(pointer_2+j+1)=(uint8_t)(HoldReg[i]&0x00FF);//再放低位
				}
					
				//写入校验码						   
				SendKey=CRC16(pointer_2+1,*pointer_2-2);					
				//将计算出来的校验码装入输出数据缓存中
				*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
				*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);
	
//				RS485_TX_EN();	
				/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
				Delay(50);
				/*	USART_IT_TXE ENABLE	*/ 
				USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
				/*	USART_IT_RXNE DISABLE	*/ 
				USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 
						
			}
			else
			{
				 ErrorHandle(3,pointer_2);//错误读取数量
			}		
		}
		else
		{
			ErrorHandle(2,pointer_2);//错误起始地址
		}
	}
	else
	{
		Comu_Busy=0;
	}
}
 /* 函数功能：预制单个寄存器
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。                                                                                      */

void PresetSingleReg(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
{
 uint16_t Address=0;//待预制寄存器的起始地址（GPIO_X,X为A，B两个端口，每个端口16位，对应地址0——31）
	uint16_t PresetValue=0;//预制数值											
	uint16_t SendKey;//要发送数据的校验值
	Address=(uint16_t)(*(pointer_1+3))*256+(*(pointer_1+4));//先得到寄存器地址
	PresetValue=(uint16_t)(*(pointer_1+5))*256+(*(pointer_1+6));//先得到预制值
	*(pointer_2+2)=0x06;//第三个字节为功能码

	if(*(pointer_1)==8)	  //如果接收到的字节数不是8个，就是一个错误帧
	{
		if(Address<100) //只要地址小于100，就是合法地址
		{
		
				*(pointer_2)=1+1+2+2+2;//有效字节个数等于丛机地址+功能码+寄存器地址+寄存器数值+CRC校验
				*(pointer_2+3)=*(pointer_1+3);//将地址值写入输出的寄存器中
				*(pointer_2+4)=*(pointer_1+4);
				*(pointer_2+5)=*(pointer_1+5);//将数值写入输出寄存器中
				*(pointer_2+6)=*(pointer_1+6);
				HoldReg[Address]=PresetValue;//将预制值写入保持寄存器
				//写入校验码
				SendKey=CRC16(pointer_2+1,*pointer_2-2);					
				//将计算出来的校验码装入输出数据缓存中
				*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
				*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);

//				RS485_TX_EN();	
				/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
				Delay(50);
				/*	USART_IT_TXE ENABLE	*/ 
				USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
				/*	USART_IT_RXNE DISABLE	*/ 
				USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 
		}
		else
		{
			ErrorHandle(2,pointer_2);//错误起始地址
		}

	}
	else
	{
		Comu_Busy=0;
	}
}
/* 函数功能：预制多个寄存器
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。                                                                                      */

void PresetMulReg(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
 {
 	uint16_t Address=0;//待预制寄存器的起始地址（HoldReg[i],i为0-99对应地址从0到99）
	uint16_t Num=0;//要预制的寄存器数量
	uint8_t  ByteCount;//预制值的字节个数
	uint16_t PresetValue=0;//预制数值											
	uint16_t SendKey;//要发送数据的校验值

	
	Address=(uint16_t)(*(pointer_1+3))*256+(*(pointer_1+4));//先得到寄存器地址
	Num=(uint16_t)(*(pointer_1+5))*256+(*(pointer_1+6));//先得到待预制寄存器数量
	*(pointer_2+2)=0x10;//第三个字节为功能码
	ByteCount= *(pointer_1+7);//表示命令值的字节数

	if((*(pointer_1)==9+ByteCount)&&ByteCount>0&&ByteCount<=200&&ByteCount==(uint8_t)(Num*2))//如果接收到的字节数不是预定的个数，或者命令字节数超出允许范围就是一个错误帧
	{

		if(Address<100) //只要地址小于100，就是合法地址
		{
		
			if(Address+Num<=100&&Num>0) //只要地址加数量大于0小于100，就是合法数量
			{
				//用于for循环
				uint8_t i;
				uint8_t j;
					
				*(pointer_2)=1+1+2+2+2;//有效字节个数等于丛机地址+功能码+寄存器地址+寄存器数量+CRC校验
				*(pointer_2+3)=*(pointer_1+3);//将地址值写入输出的寄存器中
				*(pointer_2+4)=*(pointer_1+4);
				*(pointer_2+5)=*(pointer_1+5);//将数量写入输出寄存器中
				*(pointer_2+6)=*(pointer_1+6);
	
				for(i=0,j=0;i<Num;i++,j+=2)		 	
				{
					PresetValue=(uint16_t)(*(pointer_1+8+j))*256+(*(pointer_1+9+j));//先得到预制值
				 	HoldReg[Address+i]=PresetValue;//将预制值写入保持寄存器
				}
				
	
				//写入校验码
				SendKey=CRC16(pointer_2+1,*pointer_2-2);					
				//将计算出来的校验码装入输出数据缓存中
				*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
				*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);
	
//				RS485_TX_EN();	
				/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
				Delay(50);
				/*	USART_IT_TXE ENABLE	*/ 
				USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
				/*	USART_IT_RXNE DISABLE	*/ 
				USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 
	
			}
			else
			{
				 ErrorHandle(3,pointer_2);//错误读取数量
			}
	
		}
		else
		{
			ErrorHandle(2,pointer_2);//错误起始地址
		}
	}
	else
	{
		Comu_Busy=0;
	}
}


 /* 函数功能：读取所有时间表数据
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。
	                                                                                      */
void ReadAllTimeTab(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
{
	uint16_t SendKey;//要发送数据的校验值
	uint16_t *pTabTemp = 0;
	uint16_t RecordWinCntNum=0;
	*(pointer_2+2)=0x41;//第三个字节为功能码
	if(*(pointer_1)==4)	  //如果接收到的字节数不是4个，就是一个错误帧
	{
			//用于for循环
			uint8_t i=0;
			uint8_t j=0;
			RecordWinCntNum = 0;
			for(i = 0; i < 25; i++)
			{
				if(Get_TimeTab_num(i) != 0)
				{
					RecordWinCntNum ++;
				}
			}	
			if(RecordWinCntNum !=0 )
			{
					*(pointer_2)  =1+1+RecordWinCntNum*7+2;//有效字节个数等于丛机地址+功能码+一个时间表+CRC校验
					for(i=0,j=3;i<RecordWinCntNum;i++,j+=7)//i=Address,j=4;i<Address+Num;i++,j+=2
					{
						pTabTemp= Get_TimeTab_Time(i);
						*(pointer_2+j+0)=(uint8_t)(pTabTemp[0]>>8);//先放高位
						*(pointer_2+j+1)=(uint8_t)(pTabTemp[0]&0x00FF);//再放低位
						*(pointer_2+j+2)=(uint8_t)(pTabTemp[1]>>8);//先放高位
						*(pointer_2+j+3)=(uint8_t)(pTabTemp[1]&0x00FF);//再放低位
						*(pointer_2+j+4)=(uint8_t)(pTabTemp[2]>>8);//先放高位
						*(pointer_2+j+5)=(uint8_t)(pTabTemp[2]&0x00FF);//再放低位
						*(pointer_2+j+6)=(uint8_t)(pTabTemp[3]&0x00FF);//数据个数放低位
					}
					//写入校验码						   
					SendKey=CRC16(pointer_2+1,*pointer_2-2);					
					//将计算出来的校验码装入输出数据缓存中
					*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
					*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);

//					RS485_TX_EN();	
					/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
					//Delay(50);
					/*	USART_IT_TXE ENABLE	*/ 
					USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
					/*	USART_IT_RXNE DISABLE	*/ 
					USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 
			
			}
			else
			{
					ErrorHandle(3,pointer_2);//错误读取数量
			}
	}
	else
	{
		Comu_Busy=0;
	}
}
 /* 函数功能：数组比较函数
 	函数输入：两个指针，
				pA向数组1，
			  pB指向数组2，
			  num是数据个数，
	函数输出：1数组1等于数组2
	         2数组1不等于数组2
 */
uint8_t MyStrCmp(uint8_t *pA,uint16_t *pB)
{
	uint8_t *u8pB = 0;
	u8pB = (uint8_t *)pB;
	if(*pA == *(u8pB+1))
	{
		if(*(pA+1) == *u8pB)
		{
			if(*(pA+2) == *(u8pB+3))
			{
				if(*(pA+3) == *(u8pB+2))
				{
					if(*(pA+4) == *(u8pB+5))
					{
						if(*(pA+5) == *(u8pB+4))
						{
							return 0;
						}else return 1;
					}else return 1;
				}else return 1;
			}else return 1;
		}else return 1;
	}else return 1;
}
 /* 函数功能：读取某时间点数据
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。
 */
void ReadOneDataTab(uint8_t *pointer_1,uint8_t *pointer_2)//pointer_1用作输入，pointer_2用作输出
{
	uint16_t SendKey;//要发送数据的校验值
	uint16_t *pTabTemp = 0;
	uint16_t   Num_Temp_bf = 0;
//	uint8_t  *u8pTabTemp = 0;
	uint16_t Num=0;//要读取的寄存器个数
	uint8_t x=16;
	//用于for循环
	uint8_t i=0;
	uint8_t j=0;
	extern uint16_t RecordWinCntNum;
	extern uint16_t SaveTimeJGTab[25];
	*(pointer_2+2)=0x42;//第三个字节为功能码
	if(*(pointer_1)==10)	  //如果接收到的字节数不是个，就是一个错误帧
	{
		for(i=0;i<25;i++)
		{
			pTabTemp = Get_TimeTab_Time(i);
			if(0==MyStrCmp(pointer_1+3,pTabTemp))
			{
				x = i;
				break;
			}
		}
		if(x != 16)
		{
			Num = Get_TimeTab_num(x);
			*(pointer_2)  = 10+Num*4+2;//有效字节个数等于丛机地址+功能码+一个时间表+CRC校验
			pTabTemp= Get_TimeTab_Time(x);
			*(pointer_2+3)  =(uint8_t)(pTabTemp[0]>>8);//year
			*(pointer_2+4)  =(uint8_t)(pTabTemp[0]&0x00FF);//month
			*(pointer_2+5)  =(uint8_t)(pTabTemp[1]>>8);//date
			*(pointer_2+6)  =(uint8_t)(pTabTemp[1]&0x00FF);//hour
			*(pointer_2+7)  =(uint8_t)(pTabTemp[2]>>8);//min
			*(pointer_2+8)  =(uint8_t)(pTabTemp[2]&0x00FF);//serc
			*(pointer_2+9)  =SaveTimeJGTab[x];//x#时间点的数据时间间隔
			*(pointer_2+10) =Num;//x#时间点的数据个数
			pTabTemp = NULL;
			
			Num_Temp_bf = 0;
			//这个时间点之前有多少个数据
			for(i = 0; i < x; i++)
			{
				Num_Temp_bf += Get_TimeTab_num(i);
			}
			for(i=0,j=11;i<Num;i++,j+=4)//i=Address,j=4;i<Address+Num;i++,j+=2
			{
				pTabTemp= Get_DataTab(i + Num_Temp_bf);
				*(pointer_2+j+0)=(uint8_t)(pTabTemp[0]>>8);//0#ultrasonic_sound*100H
				*(pointer_2+j+1)=(uint8_t)(pTabTemp[0]&0x00FF);//0#ultrasonic_sound*100L
				*(pointer_2+j+2)=(uint8_t)(pTabTemp[1]>>8);//0#frequency*100(40KHz)H
				*(pointer_2+j+3)=(uint8_t)(pTabTemp[1]&0x00FF);//0#frequency*100(40KHz)L
			}
			//写入校验码						   
			SendKey=CRC16(pointer_2+1,*pointer_2-2);					
			//将计算出来的校验码装入输出数据缓存中
			*(pointer_2+(*pointer_2-1))=(uint8_t)(SendKey>>8);
			*(pointer_2+(*pointer_2))=(uint8_t)(SendKey&0x00FF);

//			RS485_TX_EN();	
			/*延时35uS,等待Max3485的DE/RE引脚信号稳定*/
			Delay(50);
			/*	USART_IT_TXE ENABLE	*/ 
			USART_ITConfig(USART1,USART_IT_TXE,ENABLE);
			/*	USART_IT_RXNE DISABLE	*/ 
			USART_ITConfig(USART1,USART_IT_RXNE,DISABLE); 		
		}	else 
		{
				ErrorHandle(3,pointer_2);//错误读取数量
		}
	}
	else
	{
		Comu_Busy=0;
	}
}
/* 函数功能：对输入的信息帧进行处理，按照功能码不同，调用不同的函数处理
 	函数输入：两个指针，pointer_1指向用来存放输入信息帧的数组，
			  pointer_2用来指向存放输出信息帧的数组（两个数组的第一个元素都用来存放信息帧的有效字节个数）
			  后面的元素按照Modbus协议组织。														    
	函数输出：无。                                                                                      */
void MessageHandle(uint8_t *pointer_in,uint8_t *pointer_out)		  
{
//	uint16_t* pDataTab;
//	uint16_t* pTimeTab;
	uint16_t year=0;
	uint8_t mon=0,day=0,hour=0,min=0,sec=0;
 	uint16_t CalKey;//计算出来的校验值
	uint16_t RcvKey;//接收到的校验值
	uint16_t Address=0;
	HaveMes=0;//清除信息位
	//获取接收到的校验值
	RcvKey=(uint16_t)*(pointer_in+(*pointer_in-1));
	RcvKey=RcvKey<<8;
	RcvKey=RcvKey|(uint16_t)*(pointer_in+(*pointer_in));

	CalKey=CRC16(pointer_in+1,*pointer_in-2);
	if(CalKey==RcvKey)
	{
		switch(*(pointer_in+2))//第二个字节为功能码
		{
			case 0x03:
			{
				HoldReg[0]=k_value;
				HoldReg[1]=RangeMAX;
				HoldReg[2]=SaveGearsValue;					
				HoldReg[3]=ConectFrqcyOnOff;
				HoldReg[4]=SaveTimeJG;
				HoldReg[5]=PowrOffTime;
//				HoldReg[6]=;
//				HoldReg[7]=;
				HoldReg[8]=calendar.w_year;					
				HoldReg[9]=calendar.w_month;					
				HoldReg[10]=calendar.w_date;					
				HoldReg[11]=calendar.hour;					
				HoldReg[12]=calendar.min;					
				HoldReg[13]=calendar.sec;
				HoldReg[14]=(uint16_t)(ultrasonic_sound*100);					
				HoldReg[15]=(uint16_t)(frequency*100);					
				
				ReadHoldingReg(pointer_in,pointer_out);//读保持寄存器
				break;
			}

			case 0x06:
			{
				PresetSingleReg(pointer_in,pointer_out);//预制单个寄存器
				Address=(uint16_t)(*(pointer_in+3))*256+(*(pointer_in+4));//先得到寄存器地址

				switch	(Address)
				{
					case 0:
					{
						k_value=HoldReg[0];
						if(k_value>=1001)
							{
								k_value = 100;				//默认K值
								bsp_WriteCpuFlash(SAVE_ADDR_k_value,(uint16_t *)&k_value, 1);
							}								
					}
					break;
					case 1:
					{
						RangeMAX=HoldReg[1];
						bsp_WriteCpuFlash(SAVE_ADDR_RangeMAX, (uint16_t *)&RangeMAX,1);							
					}
					break;
////////////////////////////////////////////////////////////////////////////////////////////////////
					case 2:
					{
						SaveGearsValue=HoldReg[2];
						if(SaveGearsValue >= 5)
						{
							SaveGearsValue  = 1;        //默认档位
						}
						else if(SaveGearsValue <= 0)
						{
							SaveGearsValue  = 1;        //默认档位
						}
						else if(SaveGearsValue == 4)	//自动挡
						{
							AutoGearFlag = 1;
							
							if(WM_IsWindow(WinPara.hWinStatus)) 	//判断窗口是否有效
							{
								WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_GAUTO);		
							}
							
						}
						else 
						{
							AutoGearFlag = 0;
							GearsValue = SaveGearsValue;
							SetGear(GearsValue);
							
							if(WM_IsWindow(WinPara.hWinStatus)) 	//判断窗口是否有效
							{
								WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_GAUTO);		
							}
						}
						bsp_WriteCpuFlash(SAVE_ADDR_SaveGearsValue, (uint16_t *)&SaveGearsValue,1);		
					
					}
					break;

					case 3:
					{
						ConectFrqcyOnOff=HoldReg[3];
						bsp_WriteCpuFlash(SAVE_ADDR_ConectFrqcyOnOff, (uint16_t *)&ConectFrqcyOnOff,1);							
					}
					break;

					case 4:
					{
						SaveTimeJG=HoldReg[4];
						bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJG, (uint16_t *)&SaveTimeJG,1);	
					}
					break;

					case 5:
					{
						PowrOffTime=HoldReg[5];	
						bsp_WriteCpuFlash(SAVE_ADDR_PowrOffTime, (uint16_t *)&PowrOffTime,1);							
					}
					
					break;
////////////////////////////////////////////////////////////////////////////////////////////////////
					case 8:
					year = HoldReg[8];
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;
					case 9:
					mon  = HoldReg[9];					
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;
					case 10:
					day  = HoldReg[10];					
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;
					case 11:
					hour = HoldReg[11];					
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;
					case 12:
					min  = HoldReg[12];					
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;
					case 13:
					sec  = HoldReg[13];					
					RTC_Set(year,mon,day,hour,min,sec); //更新时间
					break;					
					default: 
					break;
				}
			}
			break;
			case 0x10:
			{
				PresetMulReg(pointer_in,pointer_out);//预制多个寄存器
				year = HoldReg[8];
				mon  = HoldReg[9];
				day  = HoldReg[10];
				hour = HoldReg[11];
				min  = HoldReg[12];
				sec  = HoldReg[13];
				RTC_Set(year,mon,day,hour,min,sec); //更新时间
				
				break;
			}												 
			case 0x11://读设备识别码
			{
				ReadDeviceID(pointer_in,pointer_out);
				break;
			}												 
			case 0x41:
			{
				ReadAllTimeTab(pointer_in,pointer_out);
				break;
			}												 
			case 0x42:
			{
				ReadOneDataTab(pointer_in,pointer_out);
				break;
			}												 
			default: 
			{
				*(pointer_out+2)=*(pointer_in+2);//获取功能码
				ErrorHandle(1,pointer_out);//功能码错误 
			}	
				break;
		}
	}
	else
	{
		Comu_Busy=0;
	} 		
}
 /* 函数功能：Modbus专用定时器（TIM4）
 	函数输入：无。														    
	函数输出：无。		
*/
void TIM4_IRQHandler(void)															  
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);  //清除TIMx的中断待处理位:TIM 中断源
		Tim_Out=1;
		TIM_Cmd(TIM4,DISABLE);
		TIM_SetCounter(TIM4,0);
		Rcv_Complete=1;
		Rcv_Buffer[0]=Rcv_Num;
		
		if(HaveMes!=0&&Rcv_Num>3)
//		if(HaveMes!=0)
		{
		Comu_Busy=1;
		HaveMes=0;////消息位置为0，表示接受完了一帧数据，暂无数据到来。7-19 14:49修改！
		PointToRcvBuf=Rcv_Buffer;
		PointToSendBuf=Send_Buffer;
//		if(*(PointToRcvBuf+1)==0)
//		ReadDeviceID(PointToRcvBuf,PointToSendBuf);
//		else
		MessageHandle(PointToRcvBuf,PointToSendBuf);
		}
	}	
}
void USART1_IRQHandler()
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		ModRcv();//用于modbus信息接收
	}
	if((USART_GetITStatus(USART1,USART_IT_TXE)!=RESET))
	{
		USART_ClearITPendingBit(USART1,USART_IT_TXE);
		ModSend();//用于modbus信息发送
	}
}
void Delay(volatile unsigned int nCount)
{
   for(; nCount != 0; nCount--);
}

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET){}; 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}
//__use_no_semihosting was requested, but _ttywrch was referenced, 增加如下函数, 方法2
void _ttywrch(int ch)
{
ch = ch;
}
#endif 



