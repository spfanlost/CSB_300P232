
#include "bsp.h"
//	一个页面里面有2048个字节
//	前256个字节（25*5*2）存储时间管理表
//	后1792个字节（448*2*2）存储数据

//////////////////////////////////////////////////////////////
//	时间点最多存储25组
//	每个时间点数据占用5个16位数据（10个字节）
//	时间				这个时间点所对应的数据个数  数据段开始位置
//	16 08 02  12 02 15			015                    0
//	16 09 03  16 12 10			025                    16
//	16 10 10  08 07 15			100                    31
//	。。。						。。。
//////////////////////////////////////////////////////////////
//uint16_t Save_TimeTab[25][5]={{0}};//时间-存储个数

////////////////////////////////////////////////////////
//	每一个时间点最多存储数据100个，
//  多余100个会记录到下一个时间点
//	448组2个16位数据（4个字节）
//	声强			频率
//	5.46			39.78
//	1.37			32.68
//	。。。			。。。
////////////////////////////////////////////////////////
//uint16_t Save_DataTab[448][2]={{0}};//声强  频率

/* 本地变量定义 ---------------------------------------------------------*/ 
static uint16_t Save_DataTab[1024]={0};
//uint16_t Save_DataTab[1024]={0};

/* 外部变量声明 ---------------------------------------------------------*/ 
extern volatile float frequency;
extern volatile float ultrasonic_sound;
extern uint16_t TimeTabItemNum;//25
extern uint16_t DataTabItemNum;//448
extern uint16_t SaveTimeCnt;


/*******************************************************************************
* 函数名称: Add_TimeTab_time()
* 功能描述: Add_TimeTab_time，添加一个时间点数据
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: 无
********************************************************************************/
void Add_TimeTab_time(uint8_t tcnt)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	Save_DataTab[0 + tcnt] = (( uint16_t )((uint8_t)(calendar.w_year-2000))<<8)|( uint16_t )calendar.w_month;//year&month
	Save_DataTab[1 + tcnt] = (( uint16_t )(calendar.w_date)<<8)|( uint16_t )calendar.hour;//date&hour
	Save_DataTab[2 + tcnt] =  (( uint16_t )(calendar.min)<<8)|( uint16_t )calendar.sec;//min&serc
}
/*******************************************************************************
* 函数名称: Add_TimeTab_Dest()
* 功能描述: Add_TimeTab_Dest，添加一个时间点位置
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 输入参数: 2、Dest 第tcnt组时间数据开始位置 范围 0-447
* 返回参数: 无
********************************************************************************/
void Add_TimeTab_Dest(uint8_t tcnt,uint16_t Dest)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	Save_DataTab[4 + tcnt] = Dest;//min&serc
}
/*******************************************************************************
* 函数名称: Clear_TimeTab()
* 功能描述: Clear_TimeTab，清除一个时间点数据
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: 无
********************************************************************************/
void Clear_TimeTab(uint8_t tcnt)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	Save_DataTab[0 + tcnt] = 0;//年月
	Save_DataTab[1 + tcnt] = 0;//日时
	Save_DataTab[2 + tcnt] = 0;//分秒
	Save_DataTab[3 + tcnt] = 0;//Num
	Save_DataTab[4 + tcnt] = 0;//Dest
}

/*******************************************************************************
* 函数名称: TimeTabHaveData()
* 功能描述: TimeTabHaveData，时间表位置是否有值
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: TRUE（有值），FALSE（无值）
********************************************************************************/
uint8_t TimeTabHaveData(uint8_t tcnt)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	if(Save_DataTab[3 + tcnt] != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;	
	}
}
/*******************************************************************************
* 函数名称: Add_TimeTab_num()
* 功能描述: Add_TimeTab_num，添加一个时间点数据
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 输入参数: 2、num  第tcnt组时间数据有cnt个数据 范围 0-447
* 返回参数: 无
********************************************************************************/
void Add_TimeTab_num(uint8_t tcnt, uint16_t num)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	if(num>=448)num=0;
	tcnt = tcnt*5;
	Save_DataTab[3 + tcnt] = num;
}
/*******************************************************************************
* 函数名称: Get_TimeTab_Time()
* 功能描述: Get_TimeTab_Time，获取一个时间点
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: 时间点数据个数
********************************************************************************/
uint16_t* Get_TimeTab_Time(uint8_t tcnt)
{
	if(tcnt>TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	return  &Save_DataTab[tcnt];
}
/*******************************************************************************
* 函数名称: Get_TimeTab_num()
* 功能描述: Get_TimeTab_num，获取一个时间点数据个数
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: 时间点数据个数
********************************************************************************/
uint16_t Get_TimeTab_num(uint8_t tcnt)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	return Save_DataTab[3 + tcnt];
}
/*******************************************************************************
* 函数名称: Get_TimeTab_Dest()
* 功能描述: Get_TimeTab_Dest，获取一个时间点数据个数
* 输入参数: 1、tcnt 第tcnt组时间数据 范围 0-24
* 返回参数: 时间点数据开始位置
********************************************************************************/
uint16_t Get_TimeTab_Dest(uint8_t tcnt)
{
	if(tcnt>=TimeTabItemNum)tcnt=0;
	tcnt = tcnt*5;
	return Save_DataTab[4 + tcnt];
}

/*******************************************************************************
* 函数名称: Add_DataTab()
* 功能描述: Add_DataTab，添加一个时间点数据
* 输入参数: 1、dat 添加第dat数据表 范围 0-447
* 返回参数: 无
********************************************************************************/
void Add_DataTab(uint16_t dat)
{
	if(dat>=DataTabItemNum) dat=0;
	dat = dat*2 + 128;
	Save_DataTab[0 + dat] = (uint16_t)(ultrasonic_sound*100);
	Save_DataTab[1 + dat] = (uint16_t)(frequency*100);
//	Save_DataTab[0 + dat] = (uint16_t)(13.14*100);
//	Save_DataTab[1 + dat] = (uint16_t)(5.27*100);
}
/*******************************************************************************
* 函数名称: Get_DataTab()
* 功能描述: Get_DataTab，添加一个时间点数据
* 输入参数: 1、dat 添加第dat数据表 范围 0-447
* 返回参数: 无
********************************************************************************/
uint16_t* Get_DataTab(uint16_t dat)
{
	if(dat>=DataTabItemNum) dat=0;
	dat = dat*2 + 128;
	return  &Save_DataTab[dat];
}
/*******************************************************************************
* 函数名称: Clear_DataTab()
* 功能描述: Clear_DataTab,清除一个时间点数据
* 输入参数: 1、dat 清除第dat数据表 范围 0-447
* 返回参数: 无
********************************************************************************/
void Clear_DataTab(uint16_t dat)
{
	if(dat>=DataTabItemNum) dat=0;
	dat = dat*2 + 128;
	Save_DataTab[0 + dat] = 0;
	Save_DataTab[1 + dat] = 0;
}

/*******************************************************************************
* 函数名称: Clear_TimeDataTab()
* 功能描述: Clear_TimeDataTab,清除第n个时间点和这个点的数据
* 输入参数: 1、n 第n个时间点
* 返回参数: 无
********************************************************************************/
void Clear_TimeDataTab(uint16_t n)
{
	uint16_t i = 0;
	uint16_t Num = 0;
	uint16_t Dest = 0;
	
	Num = Get_TimeTab_num(n);
	Dest = Get_TimeTab_Dest(n);
	for(i=Dest; i<(Num+Dest); i++)
	{
		Clear_DataTab(i);	
	}
	Clear_TimeTab(n);
}

/*******************************************************************************
* 函数名称: MoveUp_TimeDataTab()
* 功能描述: MoveUp_TimeDataTab,清除第n个时间点和这个点的数据
* 输入参数: 1、n 第n个时间点
* 返回参数: 无
********************************************************************************/
void MoveUp_TimeDataTab(uint16_t n)
{
	uint16_t Num = 0, i = 0;
	uint16_t Dest = 0;
	uint16_t temp = 0;
	uint16_t* ptime = 0;
	
	temp = n + 1;
	if(temp < TimeTabItemNum)
	{
		Num = Get_TimeTab_num(temp);
		//若下次有数据，而且不是最后一位，移位
		if(Num > 0)
		{
			Dest = Get_TimeTab_Dest(temp);
			ptime = Get_TimeTab_Time(temp);
			
			//下一次时间往上移动一位
			temp = temp*5;
			Save_DataTab[0 + temp - 1] = ptime[0];//年月
			Save_DataTab[1 + temp - 1] = ptime[1];//日时
			Save_DataTab[2 + temp - 1] = ptime[2];//分秒
			Save_DataTab[3 + temp - 1] = Num;//Num
			Save_DataTab[4 + temp - 1] = Dest;//Dest
			
			//下一次数据往上移动Num位
			for(i=Dest; i<(Num+Dest); i++)
			{
				if(i>=DataTabItemNum) i=0;
				i = i*2 + 128;
				Save_DataTab[0 + i - Num] = Save_DataTab[0 + i];
				Save_DataTab[1 + i - Num] = Save_DataTab[1 + i];
			}
			
			//清空下一次所有数据
			Clear_TimeDataTab(temp);	
		}
	}
}


/*******************************************************************************
* 函数名称: DataTabHaveData()
* 功能描述: DataTabHaveData，数据表位置是否有值
* 输入参数: 1、tcnt 第tcnt组数据数据 范围 0-447
* 返回参数: TRUE（有值），FALSE（无值）
********************************************************************************/
uint8_t DataTabHaveData(uint16_t dat)
{
	if(dat>=DataTabItemNum) dat=0;
	dat = dat*2 + 128;
	if((Save_DataTab[0 + dat] != 0)&&(Save_DataTab[1 + dat] != 0))
	{
		return TRUE;
	}
	else
	{
		return FALSE;	
	}
}


/*******************************************************************************
* 函数名称: SaveTabRead()
* 功能描述: SaveTabRead，
* 输入参数: 
* 返回参数: 
********************************************************************************/
void SaveTabRead(void)
{
	uint16_t i = 0;
	bsp_ReadCpuFlash_HalfWord(SAVE_ADDR_Save_DataTab, Save_DataTab, 1024);//读取存储的数据
	if( (Save_DataTab[0]==0xffff)||(Save_DataTab[3]==0xffff)||(Save_DataTab[5]==0xffff)||(Save_DataTab[8]==0xffff)||
		(Save_DataTab[128]==0xffff)||(Save_DataTab[129]==0xffff)||(Save_DataTab[130]==0xffff)||(Save_DataTab[131]==0xffff) )
	{
		for(i=0;i<1024;i++)//时间和数据清零
		{
			Save_DataTab[i] = 0;
		}
		SaveTimeCnt = 0;
		bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeCnt, &SaveTimeCnt, 1);	
		bsp_WriteCpuFlash(SAVE_ADDR_Save_DataTab, Save_DataTab, 1024); //存数据		
	}
///////////////////////////////////////////////////
////测试
//	SaveTimeCnt = 24;
//	for(i = 0; i < 24; i++)
//	{
//		Add_TimeTab_time(i);
//		Add_TimeTab_num(i,3);
//		Add_TimeTab_Dest(i,(3*i));
//	}
//	for(i = 0; i < 440; i++)
//	{
//		Add_DataTab(i);
//	}
/////////////////////////////////////////////////////
//	for(i=0;i<1024;i++)
//	{
//		printf("0x%08X=%04X\r\n",SAVE_ADDR_Save_DataTab+(2*i),Save_DataTab[i]); //测试用
//	}

}

void PowerOffSave(void)
{
	GUI_Power_Off();
	bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeCnt, &SaveTimeCnt, 1);	
	bsp_WriteCpuFlash(SAVE_ADDR_Save_DataTab, Save_DataTab, 1024); //存数据		
}
