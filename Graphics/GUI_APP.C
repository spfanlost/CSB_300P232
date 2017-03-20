 /*********************************Copyright (c)*********************************                                      
  *                                             
  ******************************************************************************
  * 文件名称：GUI_APP.C
  * 描    述：声强测量仪图形界面文件
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
/*Includes --------------------------------------------------------------------*/

#include "bsp.h"
			

/* 预定义字符     ------------------------------------------------------------*/
//ultrasonic
	static const COL_PROP _aColProps[] = {  //记录页面标头 //按照标号排序
	{ "Time(H:M:S)",     100, GUI_TA_HCENTER,    LISTVIEW_CompareText}, 
	{ "Value(W/cm²)",    100, GUI_TA_HCENTER,    NULL },
	{ "Frequency(KHz)",  109, GUI_TA_HCENTER,    NULL }
	};
	static const COL_PROP _aColRecordTime[] = {  //记录页面标头 //按照标号排序
	{ "Time(Y/M/D H:M:S)",  105, GUI_TA_HCENTER,   LISTVIEW_CompareText}, 
	{ "Num",                34, GUI_TA_HCENTER,    NULL}, 
	};
	static const COL_PROP _aColRecordData[] = {  //记录页面标头 //按照标号排序
	{ "Value(W/cm²)",    75, GUI_TA_HCENTER,    LISTVIEW_CompareText },
	{ "Freq(KHz)",       60, GUI_TA_HCENTER,    NULL }
	};
/* 宏定义        -------------------------------------------------------------*/
	#define COLOR_BK         GUI_BLACK
	#define COLOR_BORDER     GUI_LIGHTGRAY
	#define COLOR_FRAME      GUI_DARKGRAY
	#define COLOR_GRID       GUI_GRAY
		
/* 本地变量定义 ---------------------------------------------------------*/ 
	WIN_PARA WinPara;  //用户使用的窗口额外数据 --全局变量
	WM_HWIN hGraphWin;
	const uint8_t BORDER_TOP       =2;//波形图表边界
	const uint8_t BORDER_BOTTOM    =2;
	const uint8_t BORDER_LEFT      =35;
	const uint8_t BORDER_RIGHT     =4;   	
	const uint8_t GRID_DIST_X      =30;
	const uint8_t GRID_DIST_Y      =20;
	
	const uint8_t SetTimePositionYMD_X =65;
	const uint8_t SetTimePositionYMD_Y =50;
	const uint8_t SetTimePositionHMS_X =65;
	const uint8_t SetTimePositionHMS_Y =110;
	
	static GRAPH_DATA_Handle  hData;     //图表数据对象句柄
	static GRAPH_SCALE_Handle hScaleV;   // 垂直刻度句柄
	
	//	前13个是声强的，中间13个是频率，最后13*3个是时间
	static uint16_t   page3_data_flash[13*5]  = {0};   //存储到flash
	
	uint16_t SaveTimeJGTab[25] = {0};
	uint16_t Temp_bf = 0;
	uint16_t num_bf = 0;
	uint16_t ClearCntNum = 0;
	
	uint16_t    bat_vtg     = 3280;
	uint16_t    new_bat_vtg	= 3280;
	uint16_t    min_bat_vtg	= 3280;
	
//	uint8_t     SetingFlag          = 0;
	uint8_t     AutoGearFlag        = 0;
	uint8_t     GearDataFlag        = 0;
	uint8_t     RecordRun0Stop1Flag = 0;
	uint16_t	LogoOnOff           = 0;
	
	uint16_t    SaveTimeJG          = 3;
	
	const uint16_t TimeTabItemNum = 25;
	const uint16_t DataTabItemNum = 448;
	
	uint8_t     SaveFlag           = 0;
	uint8_t     SaveTimeFlag       = 0;
	uint8_t     SaveDatFlag        = 0;
	uint16_t    SaveTimeCnt        = 0;
	uint16_t    SaveDatCnt         = 0;
	uint16_t    SaveTimeNum        = 0;
	uint16_t    DataTabDestPointer = 0;		//数据表位置指针
	
	char   acText2[2][8] = { {0} }; 
/* 外部变量声明 ---------------------------------------------------------*/ 
	extern volatile float frequency;
	extern volatile float ultrasonic_sound;
	extern volatile float ultrasonic_sound_max;
	extern uint16_t GearsValue;
	extern uint16_t  RangeMAX; //RangeMAX严禁为零！！！
	
/* 函数声明          ---------------------------------------------------------*/ 
	
	static void _cbStatusWin(WM_MESSAGE * pMsg);
	static void CreatStatusWin(void);
	
	static void _cbOneWin(WM_MESSAGE * pMsg);
	static void CreatOneWin(void);
	
	static void _cbTwoWin(WM_MESSAGE * pMsg);
	static void CreatTwoWin(void);
	
	static void _cbThreeWin(WM_MESSAGE * pMsg);
	static void CreatThreeWin(void);
	
	static void _cbSetWin(WM_MESSAGE * pMsg);
	static void CreatSetWin(void);
	
//	static void _cbEdit(WM_MESSAGE * pMsg);
	static void _cbRadio(WM_MESSAGE * pMsg);
	static void _cbCtrl1Win(WM_MESSAGE * pMsg); 
	static void CreatCtrl1Win(void);
	
	static void _cbCtrl2Win(WM_MESSAGE * pMsg);
	static void CreatCtrl2Win(void);
	
	static void _cbSetTimeWin(WM_MESSAGE * pMsg);
	static void CreatSetTimeWin(void);
	
	static void _cbAdminWin(WM_MESSAGE * pMsg);
	static void CreatAdminWin(void);
	
	void NumToStr(uint16_t ult_temp, uint16_t frq_temp);
	static void _cbRecordWin(WM_MESSAGE * pMsg);
	static void _cbListView0(WM_MESSAGE * pMsg);
	static void _cbListView1(WM_MESSAGE * pMsg);
	static void CreatRecordWin(void);
	

	void CreatPageStartWin(void);
	static void _cbBKWin(WM_MESSAGE * pMsg);
	static void _cbTimer(GUI_TIMER_MESSAGE * pTM);
	void MainTask(void);
	
	void GUI_Power_Off(void);
	void GUI_Power_On(void);

/*******************************************************************************
* 函数名称: _cbStatusWin()
* 功能描述:  _cbStatusWin,状态栏回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbStatusWin(WM_MESSAGE * pMsg)
{
	char text_buffer[10]={0};
	uint8_t  psent_battery = 100;
	uint16_t year=0;
	uint8_t mon=0,day=0,hour=0,min=0,sec=0;
	uint8_t i=0;
	////////////////////////////////////	
	uint16_t CntBuf = 0;
	static uint8_t	ult_Eft_cnt = 0;
	
	WM_HWIN    hText1;
	PROGBAR_Handle hProgBar;

	WM_HWIN hWin; 
	hWin = pMsg->hWin;	

	hText1		  = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1); 
	hProgBar      = WM_GetDialogItem(pMsg->hWin, GUI_ID_PROGBAR0);
	switch (pMsg->MsgId) 
	{ 
		case WM_PAINT:		//重绘背景
			GUI_SetColor(GUI_BLUE);
			GUI_FillRect(0,0,WinPara.xSizeLCD ,WinPara.yPosWin);
			GUI_SetColor(GUI_LIGHTGRAY);  //绘制电池正极
			GUI_FillRect(WinPara.xSizeLCD-10,7,WinPara.xSizeLCD-9,13);
		break;			
		case MY_MSG_RTC:	//处理显示时间的信息
			RTC_Get();
			sprintf((char *)text_buffer, "%0.4d/%0.2d/%0.2d %0.2d:%0.2d:%0.2d", 
										calendar.w_year, calendar.w_month, calendar.w_date,
										calendar.hour, calendar.min, calendar.sec);      
			TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT2),text_buffer);	/* 输出时间 */
		break; 
		case MY_MSG_RTC_SETUP:	//处理设置时间的信息
			year = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT0));
			mon  = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT1));
			day  = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT2));
			hour = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT3));
			min  = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT4));
			sec  = EDIT_GetValue(WM_GetDialogItem(WinPara.hWinSetTime, GUI_ID_EDIT5));
			RTC_Set(year,mon,day,hour,min,sec); //更新时间
		break; 
		case MY_MSG_BATTERY:	//处理电池电量的信息，1
		{
			//保证电池电量直降不升2016.10.21
			new_bat_vtg = Get_Bat_Average();
			if(new_bat_vtg >= bat_vtg)
			{
				min_bat_vtg = bat_vtg;
			}
			else
			{
				bat_vtg = new_bat_vtg;
				min_bat_vtg = new_bat_vtg;
			}
//			printf("%0.4d\r\n",new_bat_vtg);
			if(min_bat_vtg>3270) 
				{psent_battery=100; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=3270)&&(min_bat_vtg>3218)) 
				{psent_battery=90; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=3218)&&(min_bat_vtg>3160)) 
				{psent_battery=80; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=3160)&&(min_bat_vtg>3101)) 
				{psent_battery=70; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=3101)&&(min_bat_vtg>3041)) 
				{psent_battery=60; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=3041)&&(min_bat_vtg>2983)) 
				{psent_battery=50; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=2983)&&(min_bat_vtg>2925)) 
				{psent_battery=40; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=2925)&&(min_bat_vtg>2866)) 
				{psent_battery=30; PROGBAR_SetSkin(hProgBar,PROGBAR_SKIN_FLEX);}
			else if((min_bat_vtg<=2866)&&(min_bat_vtg>2808)) 
				{
					psent_battery=20;   
					PROGBAR_SetSkinClassic(hProgBar);
					PROGBAR_SetBarColor(hProgBar, 0, GUI_RED);
				}
			else if((min_bat_vtg<=2808)&&(min_bat_vtg>2750)) 
				{
					psent_battery=10; 
					PROGBAR_SetSkinClassic(hProgBar);
					PROGBAR_SetBarColor(hProgBar, 0, GUI_RED);
				}
			else if(min_bat_vtg<=2750)
				{
					psent_battery=0; 					//电池电量过低
					PROGBAR_SetSkinClassic(hProgBar);
					PROGBAR_SetBarColor(hProgBar, 0, GUI_RED);
					PROGBAR_SetValue(hProgBar,psent_battery);
					for(i=0;i<3;i++)//响三声
					{						
						GPIO_ResetBits(GPIOC,GPIO_Pin_3);
						bsp_delay_ms(200);
						GPIO_SetBits(GPIOC,GPIO_Pin_3);
						bsp_delay_ms(200);
					}
					GUI_Exec();	
					PowerOffSave();
					BacklightOff();
					TIM_Cmd(TIM5, DISABLE);  //关背光PWM TIM5
					GPIO_ResetBits(GPIOC,GPIO_Pin_2);   //power off	
				}
			PROGBAR_SetValue(hProgBar,psent_battery);
		}
		break; 
		case MY_MSG_GAUTO:
		{
			if(AutoGearFlag == 1)
			{
				switch (GearsValue)
				{
					case 1:
						TEXT_SetText(hText1,"AUTO1");	
						break;
					case 2:
						TEXT_SetText(hText1,"AUTO2");	
						break;
					case 3:
						TEXT_SetText(hText1,"AUTO3");	
						break;
					default:
						break;
				}					
			}
			else
			{
				switch (GearsValue)
				{
					case 1:
						TEXT_SetText(hText1,"   ~1~");	
						break;
					case 2:
						TEXT_SetText(hText1,"   ~2~");	
						break;
					case 3:
						TEXT_SetText(hText1,"   ~3~");	
						break;
					default:
						break;
				}					
			}
		}
		break; 
		case MY_MSG_SAVE_FLAG:
			if(RecordRun0Stop1Flag == 0)
			{
				for(i=0;i<12;i++)
				{
					page3_data_flash[12-i]=page3_data_flash[11-i];
				}
				page3_data_flash[0] = (uint16_t)(ultrasonic_sound*100);				
							
				for(i=0;i<12;i++)                      
				{
					page3_data_flash[12-i+13]=page3_data_flash[11-i+13];
				}
				page3_data_flash[0+13]=(uint16_t)(frequency*100);	
				
				for(i=0;i<12;i++)//增加时间值
				{
					page3_data_flash[12 - i+26] = page3_data_flash[11 - i+26];
					page3_data_flash[12-i+13+26] = page3_data_flash[11-i+13+26];
					page3_data_flash[12-i+26+26] = page3_data_flash[11-i+26+26];
				}				
				page3_data_flash[0+26]	  =   (( uint16_t )(calendar.min)<<8)|( uint16_t )calendar.sec;//min&serc
				page3_data_flash[13+26]   =   (( uint16_t )(calendar.w_date)<<8)|( uint16_t )calendar.hour;//date&hour
				page3_data_flash[26+26]   =   (( uint16_t )((uint8_t)(calendar.w_year-2000))<<8)|( uint16_t )calendar.w_month;//year&month
//================================================================================================						
				if(ultrasonic_sound>0.05)//有效值计数
				{
					ult_Eft_cnt++;	
					if(ult_Eft_cnt>20) ult_Eft_cnt = 20;			
				}
				else
				{
					ult_Eft_cnt = 0;
				}
//================================================================================================						
				if(ult_Eft_cnt >= 2)//如果有2个以及以上有效数据(不为0) 就开始储数据了
				{
					SaveFlag ++;
					if(SaveFlag >= 2) SaveFlag = 2;
				}
				else
				{
					//停止记录
					SaveFlag = 0;
				}
//================================================================================================						
				//开始一次记录
				if(SaveFlag != 0)
				{
					//开始时间记录一次
					if(SaveFlag == 1)SaveTimeFlag = 1;
					if(SaveTimeFlag == 1)
					{
						SaveTimeFlag = 0;
						//若时间表里面有数据
						if(TimeTabHaveData(SaveTimeCnt) == TRUE)
						{
							//判断下一次有没有
							Temp_bf = SaveTimeCnt + 1;
							if(Temp_bf>=TimeTabItemNum)Temp_bf = 0;
							//若时间表下一个还有值就清空本次
							if(TimeTabHaveData(Temp_bf) == TRUE)
							{
								//清除一个时间点的时间和数据，位置不清除
								DataTabDestPointer = Get_TimeTab_Dest(SaveTimeCnt);
								Clear_TimeDataTab(SaveTimeCnt);
								Add_TimeTab_Dest(SaveTimeCnt,DataTabDestPointer);
								SaveTimeJGTab[SaveTimeCnt] = SaveTimeJG;
							}
							else
							{
								//若时间表下一个没有值就添加到下一个
								SaveTimeCnt ++;
								if(SaveTimeCnt >= TimeTabItemNum) SaveTimeCnt = 0;
								//这个时间点之前有多少个数据
								DataTabDestPointer = 0;
								for(i = 0; i < SaveTimeCnt; i++)
								{
									DataTabDestPointer += Get_TimeTab_num(i);
								}							
								Add_TimeTab_Dest(SaveTimeCnt,DataTabDestPointer);
								SaveTimeJGTab[SaveTimeCnt] = SaveTimeJG;
							}
						}
						else
						{
							//这个时间点之前有多少个数据
							DataTabDestPointer = 0;
							for(i = 0; i < SaveTimeCnt; i++)
							{
								DataTabDestPointer += Get_TimeTab_num(i);
							}
							Add_TimeTab_Dest(SaveTimeCnt,DataTabDestPointer);
							SaveTimeJGTab[SaveTimeCnt] = SaveTimeJG;
						}
						Add_TimeTab_time(SaveTimeCnt);
						//数据记录打开
						SaveDatFlag = 1;
					}
				}
				else
				{
					//数据记录关闭
					SaveDatFlag = 0;
					if(SaveDatCnt >= 1)
					{
						//添加本次记录的数据个数
						Add_TimeTab_num(SaveTimeCnt,SaveDatCnt);
						SaveDatCnt = 0;
					}
				}
				if(SaveDatFlag == 1)
				{
					DataTabDestPointer = Get_TimeTab_Dest(SaveTimeCnt);
					Add_DataTab(SaveDatCnt + DataTabDestPointer);
					SaveDatCnt++;

					//判断下一个数据位有没有值
					Temp_bf = SaveDatCnt + DataTabDestPointer;
					if(Temp_bf >= DataTabItemNum)
					{
						//数据段满了
						//停止记录一次
						SaveFlag = 0;
						ult_Eft_cnt = 0;
						//数据段满了把位置指到0
						DataTabDestPointer = 0;
					}
					//若数据表下一个还有值，就清空下一次
					if(DataTabHaveData(Temp_bf) == TRUE)
					{
						//记录清空次数
						ClearCntNum++;
						CntBuf = SaveTimeCnt+ClearCntNum;
						if(CntBuf>=TimeTabItemNum)CntBuf = 0;
						Clear_TimeDataTab(CntBuf);
					}
					if(SaveDatCnt >= 80) //一个时间点超过80个数据就停一次，记录到下一个时间点
					{
						//停止记录一次
						SaveFlag = 0;
						ult_Eft_cnt = 0;
					}
				}
//================================================================================================
			}
			else
			{
			//停止记录
			SaveFlag = 0;
			ult_Eft_cnt = 0;			
			}
		break; 
		default:		
		WM_DefaultProc(pMsg); 	
		break; 		 
	}//1
}

/*******************************************************************************
* 函数名称: CreatStatusWin()
* 功能描述: 创建顶部状态栏--用于显示标题和时间
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatStatusWin(void)
{
	WM_HWIN hText;
	PROGBAR_Handle hProgBar;
	char text_buffer[10]={0};
	WinPara.hWinStatus = WM_CreateWindowAsChild(
												0,				//窗口位置、大小
												0,
												WinPara.xSizeLCD,
												WinPara.yPosWin,	
												WM_HBKWIN, WM_CF_SHOW | WM_CF_MEMDEV, _cbStatusWin, sizeof(WIN_PARA *)
												);	
	/* 顶部的 "logo "文本大小 */
	hText = TEXT_CreateEx(5, 0, 88 , 20, WinPara.hWinStatus, WM_CF_SHOW, TEXT_CF_LEFT|TEXT_CF_VCENTER,GUI_ID_TEXT0, "Sound Power");
	TEXT_SetFont(hText,  GUI_FONT_16B_ASCII);
	TEXT_SetTextColor(hText, GUI_WHITE);

	//"AUTO"
	hText=TEXT_CreateEx(96, 0, 48, 20,WinPara.hWinStatus, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT1,"");
	TEXT_SetBkColor(hText,GUI_INVALID_COLOR);
	TEXT_SetFont(hText,GUI_FONT_16B_ASCII);
	TEXT_SetTextColor(hText, GUI_RED);
												
	/* 状态栏的电池电量显示 */
	PROGBAR_SetDefaultSkin  (PROGBAR_SKIN_FLEX);
	hProgBar = PROGBAR_CreateEx(WinPara.xSizeLCD-44,3,34,14,WinPara.hWinStatus, WM_CF_SHOW,PROGBAR_CF_HORIZONTAL,GUI_ID_PROGBAR0);
	PROGBAR_SetValue(hProgBar,100);	//初始化电池电量为0
																					
	/* 状态栏的时间显示文本 */
	hText = TEXT_CreateEx(WinPara.xSizeLCD-170,0,126,20,WinPara.hWinStatus,WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER,GUI_ID_TEXT2,"");
	TEXT_SetBkColor(hText,GUI_INVALID_COLOR);
	TEXT_SetTextColor(hText,GUI_WHITE);
	TEXT_SetFont(hText, GUI_FONT_16B_ASCII);
	sprintf((char *)text_buffer, "%0.4d/%0.2d/%0.2d %0.2d:%0.2d:%0.2d", 
								calendar.w_year, calendar.w_month, calendar.w_date,
								calendar.hour, calendar.min, calendar.sec);      
	TEXT_SetText(hText,text_buffer);	/* 输出时间 */																						
	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbOneWin()
* 功能描述:  _cbOneWin,主窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbOneWin(WM_MESSAGE * pMsg)
{
	WM_HWIN hWin;
	WM_HWIN hText;
	uint16_t i = 0;
	uint16_t temp_data = 0;
	char    text_buffer[10]={0};
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
			/* 设置聚焦 */
			WM_SetFocus(hWin);			 
		break;
		case WM_TIMER:	
			ultrasonic_sound_max = 0;		
			WM_RestartTimer(pMsg->Data.v, 60000); //重新启动定时器
		break;
		case WM_PAINT:	              //重绘背景	
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;
		case WM_KEY:
			switch (((WM_KEY_INFO *)(pMsg->Data.p))->Key) 
			{
				case GUI_KEY_UP_LONG: //长按进入页面
					WM_DeleteWindow(WinPara.hWinOne);
					CreatSetWin();
				break;
				case GUI_KEY_ESC_D_LONG: 
					CreatAdminWin();
				break;
				case GUI_KEY_ENTER_LONG://长按恢复出厂参数并关机
				{
					//默认参数
					temp_data = 100;			   //默认K值
					bsp_WriteCpuFlash(SAVE_ADDR_k_value,(uint16_t *)&temp_data, 1);
					temp_data = 1;                   //默认档位
					bsp_WriteCpuFlash(SAVE_ADDR_SaveGearsValue,(uint16_t *)&temp_data, 1);
					temp_data = 0;                   //声强计算时候是否关联频率   默认关闭
					bsp_WriteCpuFlash(SAVE_ADDR_ConectFrqcyOnOff,(uint16_t *)&temp_data, 1);
					temp_data = 3;                   //默认存储时间间隔s
					bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJG,(uint16_t *)&temp_data, 1);
					temp_data = 10;                   //关机时间  默认10
					bsp_WriteCpuFlash(SAVE_ADDR_PowrOffTime,(uint16_t *)&temp_data, 1);
					for(i=0;i<(13*2);i++)//第三页声强频率清零
					{
						page3_data_flash[i] = 0;
					}
					bsp_WriteCpuFlash(SAVE_ADDR_Page3_data_flash, page3_data_flash, (13*5));
				
					for(i=0;i<TimeTabItemNum;i++)
					{
						SaveTimeJGTab[i] = 3;
					}
					bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJGTab, SaveTimeJGTab, TimeTabItemNum);
					
					SaveTimeCnt = 0;
					for(i=0;i<TimeTabItemNum;i++)//时间和数据清零
					{
						Clear_TimeDataTab(i);
					}
					PowerOffSave();
					
					BacklightOff();
					TIM_Cmd(TIM5, DISABLE);  //关背光PWM TIM5
					GPIO_ResetBits(GPIOC,GPIO_Pin_2);   //power off		
				}
				break;				
				case GUI_KEY_LEFT: 
					WM_DeleteWindow(hWin);
					CreatThreeWin();
				break;
				case GUI_KEY_RIGHT: 
					WM_DeleteWindow(hWin);
					CreatTwoWin();
				break;				
				default:	
				break;	 
			}
		break;
		case MY_MSG_FREQUENCY:	//处理
			hText = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT3);
			sprintf((char *)text_buffer, "Fr:%5.2f",frequency );   																								
			TEXT_SetText(hText,text_buffer);	
		break;
		case MY_MSG_ULTRASONIC:	//处理
			hText = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0);
			sprintf((char *)text_buffer, "%4.2f",ultrasonic_sound);
			if(ultrasonic_sound >= 10.00f)
			{					
				TEXT_SetFont(hText, GUI_FONT_D64);					
			}
			else
			{
				TEXT_SetFont(hText, GUI_FONT_D80);
			}
			TEXT_SetText(hText,text_buffer);	
			hText = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT2);
			sprintf((char *)text_buffer, "Max:%4.2f",ultrasonic_sound_max );   																								
			TEXT_SetText(hText,text_buffer);	
		break; 			
		default:
			WM_DefaultProc(pMsg);
		break;
	} //1
}

/*******************************************************************************
* 函数名称: CreatOneWin()
* 功能描述: CreatOneWin，创建第一个显示界面
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatOneWin(void)
{
	WM_HWIN hText;
	char text_buffer[10]={0};
	//建立桌面窗口的子窗口--记录窗口
	WinPara.hWinOne = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											WinPara.yPosWin ,	//位置
											WinPara.xSizeWin,
											WinPara.ySizeWin,	//底部剩余宽度
											WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbOneWin, sizeof(WIN_PARA *)
											);	
	WM_CreateTimer(WinPara.hWinOne, 0, 60000, 0); //创建一个窗口定时器
	hText = TEXT_CreateEx(30, 60, 215 , 80, WinPara.hWinOne, WM_CF_SHOW, TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT0, " ");
	TEXT_SetTextColor(hText, GUI_RED);																																														
	sprintf((char *)text_buffer, "%4.2f",ultrasonic_sound);   
	if(ultrasonic_sound >= 10.00)
	{					
		TEXT_SetFont(hText, GUI_FONT_D64);					
	}
	else
	{
		TEXT_SetFont(hText, GUI_FONT_D80);
	}
	TEXT_SetText(hText,text_buffer);											
	/* 首页声强单位 */																						
	hText=TEXT_CreateEx(246, 98, 72, 24,WinPara.hWinOne, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT1,"W/cm²");
	TEXT_SetFont(hText, GUI_FONT_24B_1);
	TEXT_SetTextColor(hText, GUI_RED);											
	/* 最大值 */
	hText =TEXT_CreateEx(25, 186, 144, 32,WinPara.hWinOne, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT2,"");	
	TEXT_SetFont(hText, GUI_FONT_32B_ASCII);
	TEXT_SetTextColor(hText, GUI_DARKGREEN);
	sprintf((char *)text_buffer, "Max:%4.2f",ultrasonic_sound_max );   																								
	TEXT_SetText(hText,text_buffer);												
	/* 频率值 */																						
	hText =TEXT_CreateEx(178, 186, 160, 32,WinPara.hWinOne, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT3,"");	
//	hText =TEXT_CreateEx(165, 186, 160, 32,WinPara.hWinOne, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT3,"");	
	TEXT_SetFont(hText, GUI_FONT_32B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLUE);
	sprintf((char *)text_buffer, "Fr:%5.2f",frequency );   																								
	TEXT_SetText(hText,text_buffer);
	
//	/* 频率单位 */																							
//	hText=TEXT_CreateEx(270, 190, 40, 24,WinPara.hWinOne, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT4,"KHz");
//	TEXT_SetFont(hText, GUI_FONT_24B_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE);

	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbTwoWin()
* 功能描述:  _cbTwoWin,波形窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbTwoWin(WM_MESSAGE * pMsg)
{
	float  Add_Data=0;      
	char    text_buffer[10]={0};
	WM_HWIN hWin; 
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
	case WM_CREATE:
		/* 设置聚焦 */
		WM_SetFocus(hWin);
	break;
	case WM_TIMER:	
		ultrasonic_sound_max = 0;
		WM_RestartTimer(pMsg->Data.v, 60000); //重新启动定时器
	break;
	case WM_PAINT:	              //重绘背景
		GUI_SetColor(GUI_LIGHTGRAY);
		GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
	break;	
	case MY_MSG_ULTRASONIC:	//处理消息
		sprintf((char *)text_buffer, "Max:%4.2f",ultrasonic_sound_max );   		
		TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT0),text_buffer);	
		sprintf((char *)text_buffer, "Val:%4.2f",ultrasonic_sound );  
		TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT1),text_buffer);							
		switch (GearsValue)
		{
			case 1:
				GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.0000625f)); //设置刻度因数
//				GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/160); //设置刻度因数
				break;
			case 2:
				GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.00003125f)); //设置刻度因数
//				GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/2/160); //设置刻度因数
				break;
			case 3:
				GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.00000625f)); //设置刻度因数
//				GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/10/160); //设置刻度因数
				break;
			default:
				break;
		}	
	break; 			
	case WM_KEY:
		switch (((WM_KEY_INFO *)(pMsg->Data.p))->Key) 
		{
		case GUI_KEY_LEFT: 	
			WM_DeleteWindow(hWin);
			CreatOneWin();
		break;
		case GUI_KEY_RIGHT: 
			WM_DeleteWindow(hWin);
			CreatThreeWin();
		break;
		default:
		break;
		}
	break;
	case MY_MSG_ADD_GDATA :  //处理消息 向图表中增加数据
		switch (GearsValue)
		{
		case 1:		
			Add_Data=(float)(ultrasonic_sound*160/(RangeMAX*0.01f)); 	
//			Add_Data=(float)ultrasonic_sound*160/(RangeMAX/100); 	
		break;
		case 2:
			Add_Data=(float)(ultrasonic_sound*160/(RangeMAX*0.005f));
//			Add_Data=(float)ultrasonic_sound*160/(RangeMAX/2/100);
		break;
		case 3:
			Add_Data=(float)(ultrasonic_sound*160/(RangeMAX*0.001f));
//			Add_Data=(float)ultrasonic_sound*160/(RangeMAX/10/100);
		break;
		default:
		break;
		}
		GRAPH_DATA_YT_AddValue(hData, Add_Data);
		
		if(GearDataFlag == 1)//换档位要清除一下波形数据
		{
			GearDataFlag = 0;	
			GRAPH_DATA_YT_Clear(hData);
			GUI_Delay(10);
		}
	break;		 
	default:		
		WM_DefaultProc(pMsg);
	break;		
	} //1
}

/*******************************************************************************
* 函数名称: CreatTwoWin()
* 功能描述: CreatTwoWin，创建第二个显示界面
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatTwoWin(void)
{
	WM_HWIN hText;
	char text_buffer[10]={0};
	WinPara.hWinTwo = WM_CreateWindowAsChild(
											WinPara.xPosWin ,											
											WinPara.yPosWin ,	//位置
											WinPara.xSizeWin,
											WinPara.ySizeWin,	//底部剩余宽度
											WM_HBKWIN, WM_CF_SHOW | WM_CF_MEMDEV, _cbTwoWin, sizeof(WIN_PARA *)
											);		
											
	/* 窗口自动使用存储设备，防止图像变化过程中背景闪烁 */																					
	WM_SetCreateFlags(WM_CF_MEMDEV);
	WM_CreateTimer(WinPara.hWinTwo, 0, 60000, 0); //创建一个窗口定时器
											
	//创建图表
	hGraphWin= GRAPH_CreateEx(5, 5, 310, 185, WinPara.hWinTwo, WM_CF_SHOW, 0, GUI_ID_GRAPH0);	
	WIDGET_SetEffect(hGraphWin, &WIDGET_Effect_None);
	//设置边界
	GRAPH_SetBorder(hGraphWin, BORDER_LEFT, BORDER_TOP, BORDER_RIGHT, BORDER_BOTTOM); 
											
	//WM_SetHasTrans (hGraphWin);
	GRAPH_SetColor (hGraphWin, COLOR_BK,     GRAPH_CI_BK);
	GRAPH_SetColor (hGraphWin, COLOR_BORDER, GRAPH_CI_BORDER);
	GRAPH_SetColor (hGraphWin, COLOR_FRAME,  GRAPH_CI_FRAME);
	GRAPH_SetColor (hGraphWin, COLOR_GRID,   GRAPH_CI_GRID);
	
	//调整网格
	GRAPH_SetGridDistX(hGraphWin, GRID_DIST_X);
	GRAPH_SetGridDistY(hGraphWin, GRID_DIST_Y);
	GRAPH_SetGridVis  (hGraphWin, 1);
	GRAPH_SetGridFixedX(hGraphWin, 1);
	GRAPH_SetLineStyleH(hGraphWin,GUI_LS_DOT);                 //设置网格线style
	GRAPH_SetLineStyleV(hGraphWin,GUI_LS_DOT);                 //设置网格线style
	
	//GRAPH_SetUserDraw      (hItem, _UserDraw);
	//GRAPH_SetAutoScrollbar(hGraphWin, GUI_COORD_X, 1);
	
	//创建刻度
	hScaleV = GRAPH_SCALE_Create(0, GUI_TA_LEFT, GRAPH_SCALE_CF_VERTICAL, 20);
	GRAPH_SCALE_SetTextColor(hScaleV, GUI_DARKMAGENTA);
	switch (GearsValue)
	{
		case 1:
			GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.0000625f)); //设置刻度因数
//			GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/160); //设置刻度因数
			break;
		case 2:
			GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.00003125f)); //设置刻度因数
//			GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/2/160); //设置刻度因数
			break;
		case 3:
			GRAPH_SCALE_SetFactor(hScaleV, (float)(RangeMAX*0.00000625f)); //设置刻度因数
//			GRAPH_SCALE_SetFactor(hScaleV, RangeMAX/100/10/160); //设置刻度因数
			break;
		default:
			break;
	}

	GRAPH_SCALE_SetNumDecs      (hScaleV, 2);//刻度设置为x位小数
	GRAPH_SCALE_SetOff          (hScaleV,2); //设置垂直轴上下偏移位置  向上偏移可以显示负值！FRAMEWIN_CF_MOVEABLE
	GRAPH_SCALE_SetTextColor    (hScaleV, GUI_BLUE);
	GRAPH_AttachScale           (hGraphWin,hScaleV);

	//创建数据项目
	hData = GRAPH_DATA_YT_Create(GUI_GREEN, 270, 0, 0);
	GRAPH_AttachData(hGraphWin, hData); //放到图表中

	hText =TEXT_CreateEx(25, 186, 144, 32,WinPara.hWinTwo, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT0,"");	
	TEXT_SetFont(hText, GUI_FONT_32B_ASCII);
	TEXT_SetTextColor(hText, GUI_DARKGREEN);
	sprintf((char *)text_buffer, "Max:%4.2f",ultrasonic_sound_max );   																								
	TEXT_SetText(hText,text_buffer);			

	hText =TEXT_CreateEx(185, 186, 160, 32,WinPara.hWinTwo, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT1,"");	
	TEXT_SetFont(hText, GUI_FONT_32B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLUE);
	sprintf((char *)text_buffer, "Val:%4.2f",ultrasonic_sound );   																								
	TEXT_SetText(hText,text_buffer);			
	GUI_Exec();		
}
/*******************************************************************************
* 函数名称: _cbThreeWin()
* 功能描述: _cbThreeWin 记窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbThreeWin(WM_MESSAGE * pMsg) 
{
	uint8_t i = 0;
 	uint16_t temp = 0;
	const char * pText[3] = {0};
	char  acText[3][10]   = {{0}}; 
	WM_HWIN hWin;
	TEXT_Handle        hItemID_Text0;
	LISTVIEW_Handle    hItemID_ListView0;    //  创建记录表句柄
	
	hWin = pMsg->hWin;
	hItemID_ListView0 =  WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0);
	hItemID_Text0 =      WM_GetDialogItem(hWin, GUI_ID_TEXT0);
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
			WM_SetFocus(hWin);
		break;
		case WM_PAINT:	              //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;	 
		case MY_MSG_P3_STOP:	
			if(RecordRun0Stop1Flag == 0)
			{	
				TEXT_SetText(hItemID_Text0,"    ");					
			}			
			else
			{
				TEXT_SetText(hItemID_Text0,"stop");	
			}
		break;	 
		case WM_KEY:
			switch (((WM_KEY_INFO *)(pMsg->Data.p))->Key) 
			{
				case GUI_KEY_ESCAPE:
				break;
				case GUI_KEY_ENTER:
					RecordRun0Stop1Flag=!RecordRun0Stop1Flag; //记录暂停或继续标志
					WM_SendMessageNoPara(hWin,MY_MSG_P3_STOP); /*	发送消息 */			
				break;
				case GUI_KEY_LEFT:
					WM_DeleteWindow(hWin);
					CreatTwoWin();
				break;
				case GUI_KEY_RIGHT:
					WM_DeleteWindow(hWin);
					CreatOneWin();
				break;
				case GUI_KEY_UP:
					RecordRun0Stop1Flag = 1;//停止数据存储
					WM_DeleteWindow(hWin);
					CreatRecordWin();
				break; 
				
				case GUI_KEY_DOWN: 
					RecordRun0Stop1Flag = 1;//停止数据存储
					WM_DeleteWindow(hWin);
					CreatRecordWin();
				break;
				default:
				break;
			}
			break;
		case MY_MSG_ADD_DELETE:	//处理信息
			if(RecordRun0Stop1Flag == 0)
			{
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				LISTVIEW_DeleteRow(hItemID_ListView0, 0);	//删除一行
				sprintf((char *)acText[0], "%0.2d:%0.2d:%0.2d", calendar.hour, calendar.min, calendar.sec); //增加时间值
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				temp = ultrasonic_sound*100;
				if(temp/1000 != 0) acText[1][0] = (char)((uint16_t)temp/1000)+'0';
				else acText[1][0] = ' ';
				acText[1][1] = (char)((uint16_t)temp%1000/100)+'0';
				acText[1][2] = '.';
				acText[1][3] = (char)((uint16_t)temp%100/10)+'0';
				acText[1][4] = (char)((uint16_t)temp%100%10)+'0';
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				temp = frequency*100;
				if(temp/10000 != 0) acText[2][0] = (char)((uint16_t)temp/10000)+'0';				
				else acText[2][0] = ' ';
				if(temp/100<100)
				{
					if(temp%10000/1000 != 0) acText[2][1] = (char)((uint16_t)temp%10000/1000)+'0';
					else acText[2][1] = ' ';
				}
				else
				{
					acText[2][1] = (char)((uint16_t)temp%10000/1000)+'0';
				}
				acText[2][2] = (char)((uint16_t)temp%1000/100)+'0';
				acText[2][3] = '.';
				acText[2][4] = (char)((uint16_t)temp%100/10)+'0';
				acText[2][5] = (char)((uint16_t)temp%100%10)+'0';
				for (i = 0; i < 3; i++) 
				{
					pText[i] = acText[i];
				}
				LISTVIEW_AddRow(hItemID_ListView0, (const GUI_ConstString *)pText);//添加一行
			}
		break;
		default:		
		WM_DefaultProc(pMsg);
		break; 
	} //1
}
/*******************************************************************************
* 函数名称: CreatThreeWin()
* 功能描述: CreatThreeWin，创建第三个显示界面
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatThreeWin(void)
{
	uint8_t    i=0;
	uint8_t    r=0;
	uint16_t temp =0;
	const char * pText[3] = {0};
	char   acText[3][10] = { {0} }; 

	TEXT_Handle hText;
	LISTVIEW_Handle hListView;	
	//建立桌面窗口的子窗口--记录窗口
	WinPara.hWinThree = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbThreeWin, sizeof(WIN_PARA *)
											);	
	hText=TEXT_CreateEx(5, 202, 310, 16,WinPara.hWinThree, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0,"");
	TEXT_SetFont(hText,GUI_FONT_16B_ASCII);
																		
	//建立数据表控件
	hListView = LISTVIEW_CreateEx(5, 5,310, 197,WinPara.hWinThree, WM_CF_SHOW,0, GUI_ID_LISTVIEW0);
	WIDGET_SetEffect(hListView, &WIDGET_Effect_None);
//	HEADER_SetDragLimit(LISTVIEW_GetHeader(hListView ), 1); // 将拖动分割线限制为打开
	LISTVIEW_SetAutoScrollH(hListView , 0); //禁用用自动使用水平滚动条
	LISTVIEW_SetAutoScrollV(hListView , 0); //禁用用自动使用水平滚动条
	LISTVIEW_SetGridVis(hListView , 1);     //设置网格线可见
	for (i = 0; i < 3; i++) 
	{
		LISTVIEW_AddColumn(hListView , _aColProps[i].Width, _aColProps[i].pText, _aColProps[i].Align);
		LISTVIEW_SetCompareFunc(hListView , i, _aColProps[i].fpCompare);
	}
	LISTVIEW_EnableSort(hListView); //启用排序功能
	for(r=0;r<13;r++) //添加内容 
	{
		sprintf((char *)acText[0], "%0.2d:%0.2d:%0.2d",(page3_data_flash[12-r+13+26]&0x00ff), 
													   (page3_data_flash[12-r+26]>>8), 
													   (page3_data_flash[12-r+26]&0x00ff) ); //增加时间值
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		temp = page3_data_flash[12-r];
		if(temp/1000 != 0) acText[1][0] = (char)((uint16_t)temp/1000)+'0';
		else acText[1][0] = ' ';
		acText[1][1] = (char)((uint16_t)temp%1000/100)+'0';
		acText[1][2] = '.';
		acText[1][3] = (char)((uint16_t)temp%100/10)+'0';
		acText[1][4] = (char)((uint16_t)temp%100%10)+'0';
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		temp = page3_data_flash[12-r+13];
		if(temp/10000 != 0) acText[2][0] = (char)((uint16_t)temp/10000)+'0';				
		else acText[2][0] = ' ';
		if(temp/100<100)
		{
			if(temp%10000/1000 != 0) acText[2][1] = (char)((uint16_t)temp%10000/1000)+'0';
			else acText[2][1] = ' ';
		}
		else
		{
			acText[2][1] = (char)((uint16_t)temp%10000/1000)+'0';
		}
		acText[2][2] = (char)((uint16_t)temp%1000/100)+'0';
		acText[2][3] = '.';
		acText[2][4] = (char)((uint16_t)temp%100/10)+'0';
		acText[2][5] = (char)((uint16_t)temp%100%10)+'0';		
		for (i = 0; i < 3; i++) 
		{
			pText[i] = acText[i];
		}
		LISTVIEW_AddRow(hListView, (const GUI_ConstString *)pText);
	}
	GUI_Exec();	
	WM_SendMessageNoPara(WinPara.hWinThree,MY_MSG_P3_STOP); /*	发送消息 */			
}
/*******************************************************************************
* 函数名称: _cbSetWin()
* 功能描述: _cbSetWin 密码窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbSetWin(WM_MESSAGE * pMsg) 
{
	WM_HWIN hWin;
	int indx = 0;
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
		break;
		case WM_PAINT:	              //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;
		case WM_KEY:
			switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
			{
				case GUI_KEY_ESCAPE:
					WM_DeleteWindow(hWin);
					CreatOneWin();
				break;
				case GUI_KEY_ENTER:
					indx = ICONVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_ICONVIEW0));					
					if(indx==0)
					{
						WM_DeleteWindow(hWin);
						CreatCtrl1Win();
					}
					else if(indx==1)
					{
						WM_DeleteWindow(hWin);
						CreatCtrl2Win();											
					}
					else if(indx==2)
					{
						WM_DeleteWindow(hWin);
						CreatSetTimeWin();											
					}
					else
					{
						WM_DeleteWindow(hWin);
						CreatOneWin();
					}
				break;
				default:	
				break;	 
			}
		break;			
		default:		
		WM_DefaultProc(pMsg);	
		break;	 
	} //1
}
/*******************************************************************************
* 函数名称: CreatSetWin()
* 功能描述: CreatSetWin
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatSetWin(void)
{
	WM_HWIN hText;
	ICONVIEW_Handle hIcoview;
	//建立桌面窗口的子窗口--密码界面
	WinPara.hWinSet = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbSetWin, sizeof(WIN_PARA *)
											);	
	WM_SetCreateFlags(WM_CF_MEMDEV);
	hText=TEXT_CreateEx(20, 30, 280, 24,WinPara.hWinSet, WM_CF_SHOW,GUI_TA_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0,"Settings Page");
	TEXT_SetFont(hText, GUI_FONT_24B_ASCII);
	TEXT_SetTextColor(hText, GUI_RED);
	hIcoview = ICONVIEW_CreateEx(60,80,200,110,WinPara.hWinSet,WM_CF_SHOW,0,GUI_ID_ICONVIEW0,60,90);
	ICONVIEW_SetTextColor(hIcoview,0,GUI_BLACK);
	ICONVIEW_SetBkColor(hIcoview,0,GUI_LIGHTGRAY);
	ICONVIEW_SetFont(hIcoview,GUI_FONT_20_ASCII);
	ICONVIEW_AddBitmapItem(hIcoview,&bmConf1,"Conf1");
	ICONVIEW_AddBitmapItem(hIcoview,&bmConf2,"Conf2");
	ICONVIEW_AddBitmapItem(hIcoview,&bmClock,"Clock");													  
	/* 设置聚焦 */
	WM_SetFocus(hIcoview);										  											  
	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbCtrl1Win()
* 功能描述: _cbCtrl1Win 控制窗口1回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbCtrl1Win(WM_MESSAGE * pMsg) 
{
	extern uint16_t k_value;
	extern uint16_t SaveGearsValue;
	WM_HWIN hWin; 
//	EDIT_Handle hObj;
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
			/* 设置聚焦 */
			WM_SetFocus(hWin);
		break;
		case WM_PAINT:	              //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
			GUI_DrawBitmap(&bmkval,75,40);
			GUI_DrawBitmap(&bmhval,75,90);
			GUI_DrawBitmap(&bmgear,75,140);
		break;		 
		case WM_KEY:
			switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
			{
				case GUI_KEY_ESCAPE:
					WM_DeleteWindow(hWin);
					CreatSetWin();
				break;
				case GUI_KEY_ENTER:
				{
					if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_EDIT0)))
					{
						k_value = EDIT_GetValue(WM_GetDialogItem(hWin, GUI_ID_EDIT0));
						bsp_WriteCpuFlash(SAVE_ADDR_k_value,(uint16_t *)&k_value, 1);		
					}
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_EDIT1)))
					{	
						RangeMAX = EDIT_GetValue(WM_GetDialogItem(hWin, GUI_ID_EDIT1));
						bsp_WriteCpuFlash(SAVE_ADDR_RangeMAX,(uint16_t *)&RangeMAX, 1);							
					}
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO0)))
					{
						SaveGearsValue=1+RADIO_GetValue(WM_GetDialogItem(hWin, GUI_ID_RADIO0));
						if(SaveGearsValue == 4)
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
					WM_SetFocusOnNextChild(hWin);
				}
				break;
//				case GUI_KEY_BUF_L:
//					PosBuf = EDIT_GetCursorCharPos(WM_GetDialogItem(hWin, GUI_ID_EDIT0));
//					PosBuf -= 1;
//					if(PosBuf<0)
//					{
//						WM_SetFocusOnPrevChild(hWin);
//					}
//					EDIT_SetCursorAtChar(WM_GetDialogItem(hWin, GUI_ID_EDIT0),PosBuf);
//				
//				break;
//				case GUI_KEY_BUF_R:
//					NumCharsBuf = EDIT_GetNumChars(WM_GetDialogItem(hWin, GUI_ID_EDIT0));
//					PosBuf = EDIT_GetCursorCharPos(WM_GetDialogItem(hWin, GUI_ID_EDIT0));
//					PosBuf += 1;
//					if(PosBuf>NumCharsBuf) 
//					{
//						WM_SetFocusOnNextChild(hWin);
//					}
//					EDIT_SetCursorAtChar(WM_GetDialogItem(hWin, GUI_ID_EDIT0),PosBuf);
//				break;
				case GUI_KEY_FOCUS_NEXT:
					WM_SetFocusOnNextChild(hWin);
				break;
				case GUI_KEY_FOCUS_PREV: 
					WM_SetFocusOnPrevChild(hWin);
				break;
				default:
				break;	 
			}
		break;			 
		default:		
		WM_DefaultProc(pMsg);	
		break;	 
	} //1
}
/*******************************************************************************
* 函数名称: _cbRadio()
* 功能描述: _cbRadio，控件回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbRadio(WM_MESSAGE * pMsg) 
{	
	const WM_KEY_INFO * pInfo;
	switch (pMsg->MsgId) 
	{
	case WM_KEY: 
		pInfo = (WM_KEY_INFO *)pMsg->Data.p;
		switch(pInfo->Key)
		{
		case GUI_KEY_LEFT:
			GUI_StoreKeyMsg(GUI_KEY_FOCUS_PREV, 1);
			break;
		case GUI_KEY_RIGHT:
			GUI_StoreKeyMsg(GUI_KEY_FOCUS_NEXT, 1);
			break;
		default:
			RADIO_Callback(pMsg); 
			break;
		}
		break;
	default:
		RADIO_Callback(pMsg); 
		break;
	}
}
/*******************************************************************************
* 函数名称: _cbEdit()
* 功能描述: _cbEdit，控件回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
//static void _cbEdit(WM_MESSAGE * pMsg) 
//{
//	const WM_KEY_INFO * pInfo;
//	switch (pMsg->MsgId) 
//	{
//	case WM_KEY: 
//		pInfo = (WM_KEY_INFO *)pMsg->Data.p;
//		switch(pInfo->Key)
//		{
//		case GUI_KEY_LEFT:
//			GUI_StoreKeyMsg(GUI_KEY_BUF_L, 1);
//			break;
//		case GUI_KEY_RIGHT:
//			GUI_StoreKeyMsg(GUI_KEY_BUF_R, 1);
//			break;
//		default:
//			EDIT_Callback(pMsg); 
//		break;
//		}
//		break;
//	default:
//	EDIT_Callback(pMsg); 
//	break;
//	}
//}

/*******************************************************************************
* 函数名称: CreatCtrl1Win()
* 功能描述: CreatCtrl1Win，创建参数设置界面
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatCtrl1Win(void)
{
	WM_HWIN hText;
	EDIT_Handle hEdit;
	RADIO_Handle hRadio;
	extern uint16_t SaveGearsValue;
	extern uint16_t k_value;
	extern uint16_t PowrOffTime ;              
	//建立桌面窗口的子窗口--参数设置界面
	WinPara.hWinCtrl1 = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbCtrl1Win, sizeof(WIN_PARA *)
											);	
	WM_SetCreateFlags(WM_CF_MEMDEV);

	hText=TEXT_CreateEx(125, 40, 56, 24,WinPara.hWinCtrl1, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT0,"K:");
	TEXT_SetFont(hText, GUI_FONT_24B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(155, 40,  54, 24,WinPara.hWinCtrl1, WM_CF_SHOW,0,GUI_ID_EDIT0,10);
//	WM_SetCallback(hEdit, _cbEdit);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,k_value,0,1000,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,3);//默认光标位置	
	WM_SetFocus(hEdit);

	hText=TEXT_CreateEx(125, 90, 56, 24,WinPara.hWinCtrl1, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT1,"H:");
	TEXT_SetFont(hText, GUI_FONT_24B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(155, 90,  60, 24,WinPara.hWinCtrl1, WM_CF_SHOW,0,GUI_ID_EDIT1,10);
	EDIT_EnableBlink(hEdit, 10, 0);
											  										  
	EDIT_SetFloatMode(hEdit, (float)(RangeMAX*0.01f), 0, 99.99, 2, GUI_EDIT_NORMAL);										  
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,4);//默认光标位置	
	hText=TEXT_CreateEx(220, 90, 80, 24,WinPara.hWinCtrl1, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT2,"W/cm²");
	TEXT_SetFont(hText, GUI_FONT_16B_1);
	
	hText=TEXT_CreateEx(125, 140, 65, 20,WinPara.hWinCtrl1, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT3,"G:");
	TEXT_SetFont(hText,GUI_FONT_24B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLUE);
											  
	hRadio = RADIO_CreateEx(155, 140, 75, 79,WinPara.hWinCtrl1, WM_CF_SHOW,0, GUI_ID_RADIO0, 4,16);		
	WM_SetCallback(hRadio, _cbRadio);
	RADIO_SetFont(hRadio, GUI_FONT_16B_ASCII);	
	RADIO_SetText(hRadio, "High", 0);
	RADIO_SetText(hRadio, "Medium", 1);
	RADIO_SetText(hRadio, "Low", 2);				
	RADIO_SetText(hRadio, "Auto", 3);	
	RADIO_SetValue(hRadio,SaveGearsValue-1);	//默认光标位置
	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbCtrl2Win()
* 功能描述: _cbCtrl2Win 窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbCtrl2Win(WM_MESSAGE * pMsg) 
{
	WM_HWIN hWin; 
	extern uint16_t ConectFrqcyOnOff;   //声强计算时候是否关联频率   默认打开	
	extern uint16_t PowrOffTime ;              
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
			/* 设置聚焦 */
			WM_SetFocus(hWin);
			//继续增加函数		 
		break;
		case WM_PAINT:	  //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
			GUI_DrawBitmap(&bmFREQ,70,35);
			GUI_DrawBitmap(&bmSAVEICO,70,85);
			GUI_DrawBitmap(&bmOFFICO,70,135);
		break;		 
		case WM_KEY:
			switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
			{
				case GUI_KEY_ESCAPE:
					WM_DeleteWindow(hWin);
					CreatSetWin();
				break;
				case GUI_KEY_ENTER:
					//频率关联
					if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO0)))
					{
						ConectFrqcyOnOff = RADIO_GetValue(WM_GetDialogItem(hWin, GUI_ID_RADIO0));
						bsp_WriteCpuFlash(SAVE_ADDR_ConectFrqcyOnOff,(uint16_t *)&ConectFrqcyOnOff, 1);
						WM_SetFocusOnNextChild(hWin);
					}
					//存储时间间隔
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO1)))
					{
						if(0 == RADIO_GetValue(WM_GetDialogItem(hWin, GUI_ID_RADIO1)))
						{
							//停止存储
							RecordRun0Stop1Flag = 1;
							SaveTimeJG = 0;
							bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJG,(uint16_t *)&SaveTimeJG, 1);
							WM_SetFocusOnNextChild(hWin);						
							WM_SetFocusOnNextChild(hWin);
						}
						else
						{
							RecordRun0Stop1Flag = 0;
							WM_SetFocusOnNextChild(hWin);
						}
					}
					//自动关机时间
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO2)))
					{
						if(0 == RADIO_GetValue(WM_GetDialogItem(hWin, GUI_ID_RADIO2)))
						{
							PowrOffTime = 0;
							bsp_WriteCpuFlash(SAVE_ADDR_PowrOffTime,(uint16_t *)&PowrOffTime, 1);
							WM_SetFocusOnNextChild(hWin);						
							WM_SetFocusOnNextChild(hWin);
						}
						else
						{
							WM_SetFocusOnNextChild(hWin);
						}
					}
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_EDIT0)))
					{
						SaveTimeJG = EDIT_GetValue(WM_GetDialogItem(hWin, GUI_ID_EDIT0));
						bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJG,(uint16_t *)&SaveTimeJG, 1);

						WM_SetFocusOnNextChild(hWin);
					}
					else if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_EDIT1)))
					{
						PowrOffTime = EDIT_GetValue(WM_GetDialogItem(hWin, GUI_ID_EDIT1));
						bsp_WriteCpuFlash(SAVE_ADDR_PowrOffTime,(uint16_t *)&PowrOffTime, 1);
						WM_SetFocusOnNextChild(hWin);
					}
					
				break;
				case GUI_KEY_FOCUS_NEXT:
					if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO0)))
					{
						WM_SetFocusOnNextChild(hWin);
					}
					else
					{
						WM_SetFocusOnNextChild(hWin);
						WM_SetFocusOnNextChild(hWin);
					}
					break;
				case GUI_KEY_FOCUS_PREV:
					if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO1)))
					{
						WM_SetFocusOnPrevChild(hWin);
					}
					else
					{
						WM_SetFocusOnPrevChild(hWin);
						WM_SetFocusOnPrevChild(hWin);
					}
					break;
				default:	break;	 
			}
		break;			 
		default:		
		WM_DefaultProc(pMsg);	break;	 
	} //1
}
/*******************************************************************************
* 函数名称: CreatCtrl2Win()
* 功能描述: CreatCtrl2Win
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatCtrl2Win(void)
{
	WM_HWIN hText;
	WM_HWIN hEdit;
	RADIO_Handle hRadio;
	extern uint16_t ConectFrqcyOnOff;                   //声强计算时候是否关联频率   默认打开
	extern uint16_t PowrOffTime ;              
	//建立桌面窗口的子窗口
	WinPara.hWinCtrl2 = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbCtrl2Win, sizeof(WIN_PARA *)
											);	
	WM_SetCreateFlags(WM_CF_MEMDEV);
	//频率关联设置
	hText=TEXT_CreateEx(125, 40, 26, 24, WinPara.hWinCtrl2, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0,"F:");
	TEXT_SetFont(hText,GUI_FONT_24B_ASCII );
	TEXT_SetTextColor(hText, GUI_BLACK);
	hRadio = RADIO_CreateEx(150, 35, 60, 60,WinPara.hWinCtrl2, WM_CF_SHOW, 0, GUI_ID_RADIO0, 2, 20);		
	WM_SetCallback(hRadio, _cbRadio);
	RADIO_SetFont(hRadio, GUI_FONT_16B_ASCII);	
	RADIO_SetText(hRadio, "NO", 0);
	RADIO_SetText(hRadio, "YES",  1);
	RADIO_SetValue(hRadio,ConectFrqcyOnOff);		
	WM_SetFocus(hRadio);//默认光标位置
	//存储时间间隔设置
	hText=TEXT_CreateEx(125, 90, 26, 24, WinPara.hWinCtrl2, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT1,"T:");
	TEXT_SetFont(hText,GUI_FONT_24B_ASCII );
	TEXT_SetTextColor(hText, GUI_BLACK);										  
	hRadio = RADIO_CreateEx(150, 85, 60, 60, WinPara.hWinCtrl2, WM_CF_SHOW, 0, GUI_ID_RADIO1, 2, 20);		
	WM_SetCallback(hRadio, _cbRadio);
	RADIO_SetFont(hRadio, GUI_FONT_16B_ASCII);	
	RADIO_SetText(hRadio, "NO", 0);
	RADIO_SetText(hRadio, "YES",  1);
	if(SaveTimeJG>=1)
	RADIO_SetValue(hRadio,1);	//默认光标位置
	else
	RADIO_SetValue(hRadio,0);	//默认光标位置
	hEdit=EDIT_CreateEx(200, 103, 36, 20,WinPara.hWinCtrl2, WM_CF_SHOW,0,GUI_ID_EDIT0,10);
	EDIT_SetDecMode(hEdit,SaveTimeJG,1,600,0,GUI_EDIT_NORMAL);//设置初值
	EDIT_SetFont(hEdit, GUI_FONT_20B_ASCII);
	EDIT_SetCursorAtChar(hEdit,2);//默认光标位置	
	hText=TEXT_CreateEx(240, 105, 10, 17,WinPara.hWinCtrl2, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT2,"s");
	TEXT_SetFont(hText,GUI_FONT_20_ASCII );
	TEXT_SetTextColor(hText, GUI_BLACK);
	//关机时间设置
	hText=TEXT_CreateEx(104, 140, 48, 24,WinPara.hWinCtrl2, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT3,"OFF:");
	TEXT_SetFont(hText,GUI_FONT_24B_ASCII );
	TEXT_SetTextColor(hText, GUI_BLACK);										  
	hRadio = RADIO_CreateEx(150, 135, 60, 60, WinPara.hWinCtrl2, WM_CF_SHOW,0, GUI_ID_RADIO2, 2,20);		
	WM_SetCallback(hRadio, _cbRadio);
	RADIO_SetFont(hRadio, GUI_FONT_16B_ASCII);	
	RADIO_SetText(hRadio, "NO", 0);
	RADIO_SetText(hRadio, "YES",  1);
	if(PowrOffTime>=1)
	RADIO_SetValue(hRadio,1);	//默认光标位置
	else
	RADIO_SetValue(hRadio,0);	//默认光标位置
	hEdit=EDIT_CreateEx(200, 153, 36, 20,WinPara.hWinCtrl2, WM_CF_SHOW,0,GUI_ID_EDIT1,10);
	EDIT_SetDecMode(hEdit,PowrOffTime,1,100,0,GUI_EDIT_NORMAL);//设置初值
	EDIT_SetFont(hEdit, GUI_FONT_20B_ASCII);
	EDIT_SetCursorAtChar(hEdit,2);//默认光标位置	
	hText=TEXT_CreateEx(240, 155, 30, 17,WinPara.hWinCtrl2, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT4,"min");
	TEXT_SetFont(hText,GUI_FONT_20_ASCII );
	TEXT_SetTextColor(hText, GUI_BLACK);
	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbSetTimeWin()
* 功能描述: _cbSetTimeWin 时间设置窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbSetTimeWin(WM_MESSAGE * pMsg) 
{
	WM_HWIN hWin; 
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_CREATE:
			/* 设置聚焦 */
			WM_SetFocus(hWin);
		break;
		case WM_PAINT://重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;
		case WM_KEY:
			switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
			{
				case GUI_KEY_ESCAPE:
					WM_DeleteWindow(hWin);
					CreatSetWin();
				break;
				case GUI_KEY_ENTER:
				{
					if(WM_IsWindow(WinPara.hWinStatus)) 	//判断窗口是否有效
					{
						if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_EDIT5))) /*发送时间设置消息 */
						{
						WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_RTC_SETUP);		
						}
					}
				WM_SetFocusOnNextChild(hWin);//指向下一个对象
				}
				break;
				default:	
				break;	 
			}
			break;
		default:		
		WM_DefaultProc(pMsg);
		break;	 
	} //1
}
/*******************************************************************************
* 函数名称: CreatSetTimeWin()
* 功能描述: CreatSetTimeWin
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatSetTimeWin(void)
{
	WM_HWIN hText;
	EDIT_Handle hEdit;
	//建立桌面窗口的子窗口--界面
	WinPara.hWinSetTime = WM_CreateWindowAsChild(WinPara.xPosWin ,											
												  WinPara.yPosWin ,	//位置
												  WinPara.xSizeWin,
												  WinPara.ySizeWin,	//底部剩余宽度
												  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbSetTimeWin, sizeof(WIN_PARA *)
												);	
	WM_SetCreateFlags(WM_CF_MEMDEV);
	hText=TEXT_CreateEx(70, 20, 180, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT6,"Set Current Time");
	TEXT_SetFont(hText, GUI_FONT_24B_ASCII);
	TEXT_SetTextColor(hText, GUI_RED);

	hText=TEXT_CreateEx(SetTimePositionYMD_X, SetTimePositionYMD_Y, 56, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT0,"Year");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(SetTimePositionYMD_X, SetTimePositionYMD_Y+20,  56,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT0,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.w_year,2000,2099,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,3);//默认光标位置	
	WM_SetFocus(hEdit);
																							
	hText=TEXT_CreateEx(SetTimePositionYMD_X+73, SetTimePositionYMD_Y, 60, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT1,"Month");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE); 
	hEdit=EDIT_CreateEx(SetTimePositionYMD_X+80, SetTimePositionYMD_Y+20, 32,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT1,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.w_month,1,12,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,1);//默认光标位置	
																							
	hText=TEXT_CreateEx(SetTimePositionYMD_X+160, SetTimePositionYMD_Y, 32, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT2,"Day");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(SetTimePositionYMD_X+160, SetTimePositionYMD_Y+20,  32,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT2,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.w_date,1,31,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,1);//默认光标位置	

	hText=TEXT_CreateEx(SetTimePositionHMS_X-2, SetTimePositionHMS_Y, 40, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT3,"Hour");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(SetTimePositionHMS_X, SetTimePositionHMS_Y+20,  32,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT3,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.hour,0,23,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,1);//默认光标位置	
																							
	hText=TEXT_CreateEx(SetTimePositionHMS_X+72, SetTimePositionHMS_Y, 60, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT4,"Minute");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE); 
	hEdit=EDIT_CreateEx(SetTimePositionHMS_X+80, SetTimePositionHMS_Y+20,  32,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT4,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.min,0,59,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,1);//默认光标位置	
																							
	hText=TEXT_CreateEx(SetTimePositionHMS_X+147, SetTimePositionHMS_Y, 60, 24,WinPara.hWinSetTime, WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT5,"Second");
	TEXT_SetFont(hText, GUI_FONT_20_ASCII);
//	TEXT_SetTextColor(hText, GUI_BLUE);
	hEdit=EDIT_CreateEx(SetTimePositionHMS_X+160, SetTimePositionHMS_Y+20,  32,  30,WinPara.hWinSetTime, WM_CF_SHOW,0,GUI_ID_EDIT5,5);
	EDIT_EnableBlink(hEdit, 10, 0);
	EDIT_SetDecMode(hEdit,calendar.sec,0,59,0,GUI_EDIT_NORMAL);
	EDIT_SetFont(hEdit, GUI_FONT_24B_ASCII);
	EDIT_SetCursorAtChar(hEdit,1);//默认光标位置	
	GUI_Exec();	
}
/*******************************************************************************
* 函数名称: _cbAdminWin()
* 功能描述: _cbAdminWin 窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbAdminWin(WM_MESSAGE * pMsg) 
{
	WM_HWIN hWin; 
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{
		case WM_CREATE:
			WM_SetFocus(hWin);
		break;
		case WM_PAINT:	              //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;		 
		case WM_KEY:
			switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
			{
				case GUI_KEY_ESCAPE:
					WM_DeleteWindow(hWin);
					CreatOneWin();
				break;
				case GUI_KEY_ENTER:
					if(WM_HasFocus(WM_GetDialogItem(hWin, GUI_ID_RADIO0)))
					{
						LogoOnOff=RADIO_GetValue(WM_GetDialogItem(hWin, GUI_ID_RADIO0));
						bsp_WriteCpuFlash(SAVE_ADDR_LogoOnOff, (uint16_t *)&LogoOnOff,1);		
					}
					WM_DeleteWindow(hWin);
					CreatOneWin();
				break;
				default:	
				break;	 
			}
		break;			 
		default:		
		WM_DefaultProc(pMsg);	break;	 
	} //1
}

/*******************************************************************************
* 函数名称: CreatAdminWin()
* 功能描述: CreatAdminWin
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatAdminWin(void)
{
	WM_HWIN hText;
	RADIO_Handle hRadio;
	//建立桌面窗口的子窗口
	WinPara.hWinAdmin = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbAdminWin, sizeof(WIN_PARA *)
											);	
	WM_SetCreateFlags(WM_CF_MEMDEV);
	hText=TEXT_CreateEx(115, 70, 100, 24,WinPara.hWinAdmin, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0,"Show Logo");
	TEXT_SetFont(hText,GUI_FONT_20_ASCII );
	TEXT_SetTextColor(hText, GUI_BLUE);											  
	hRadio = RADIO_CreateEx(140, 95, 60, 60,WinPara.hWinAdmin, WM_CF_SHOW,0, GUI_ID_RADIO0, 2,20);		
	WM_SetCallback(hRadio, _cbRadio);
	RADIO_SetFont(hRadio, GUI_FONT_16B_ASCII);	
	RADIO_SetText(hRadio, "OFF", 0);
	RADIO_SetText(hRadio, "ON",  1);
	RADIO_SetValue(hRadio,LogoOnOff);	//默认光标位置	
	WM_SetFocus(hRadio);				
	GUI_Exec();	
}


/*******************************************************************************
* 函数名称: NumToStr()
* 功能描述: NumToStr 
* 输入参数: void
* 返回参数: 无
********************************************************************************/
void NumToStr(	uint16_t ult_temp, uint16_t frq_temp)
{
	if(ult_temp/1000 != 0) acText2[0][0] = (char)((uint16_t)ult_temp/1000)+'0';
	else acText2[0][0] = ' ';
	acText2[0][1] = (char)((uint16_t)ult_temp%1000/100)+'0';
	acText2[0][2] = '.';
	acText2[0][3] = (char)((uint16_t)ult_temp%100/10)+'0';
	acText2[0][4] = (char)((uint16_t)ult_temp%100%10)+'0';
///////////////////////////////////////////////////////////////////////////////////////////////////////
	if(frq_temp/10000 != 0) 
	acText2[1][0] = (char)((uint16_t)frq_temp/10000)+'0';	 
	else 
	acText2[1][0] = ' '; 
	if(frq_temp/100<100)
	{
		if(frq_temp%10000/1000 != 0) 
		acText2[1][1] = (char)((uint16_t)frq_temp%10000/1000)+'0'; 
		else 
		acText2[1][1] = ' '; 
	}
	else
	{
		acText2[1][1] = (char)((uint16_t)frq_temp%10000/1000)+'0';		
	}
	acText2[1][2] = (char)((uint16_t)frq_temp%1000/100)+'0';
	acText2[1][3] = '.';
	acText2[1][4] = (char)((uint16_t)frq_temp%100/10)+'0';
	acText2[1][5] = (char)((uint16_t)frq_temp%100%10)+'0';		
}

/*******************************************************************************
* 函数名称: _cbRecordWin()
* 功能描述: _cbRecordWin 记窗口回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbRecordWin(WM_MESSAGE * pMsg) 
{
	uint8_t   i = 0,r = 0;
	uint16_t  t_Temp  = 0;
	uint32_t  t_Temp1 = 0;
	int inx =0;
	uint16_t   Num_Temp = 0;
	uint16_t   Num_Temp_bf = 0;
	uint16_t   NumRows = 0;
	uint16_t   *pTabTemp = 0;
	const char *pText[3];
	char text_buffer[10]={0};
	WM_HWIN hWin; 
	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{	//1
		case WM_PAINT:	              //重绘背景
			GUI_SetColor(GUI_LIGHTGRAY);
			GUI_FillRect(0,0,WinPara.xSizeWin ,WinPara.ySizeWin);
		break;	 
		case WM_KEY:
			switch (((WM_KEY_INFO *)(pMsg->Data.p))->Key) 
			{
				case GUI_KEY_ESCAPE: 
					RecordRun0Stop1Flag = 0;//开始数据存储
					WM_DeleteWindow(hWin);
					CreatThreeWin();
				break;
				
//				case GUI_KEY_ENTER:
//				break;
//								
//				case GUI_KEY_UP: 
//				break; 
//				
//				case GUI_KEY_DOWN: 
//				break;
				
				case GUI_KEY_FOCUS_NEXT: 
					WM_SetFocusOnNextChild(hWin);
				break;
				
				case GUI_KEY_FOCUS_PREV: 
					WM_SetFocusOnPrevChild(hWin);
				break;
				
				case GUI_KEY_LIST1_REFRESH_UP: 
					LISTVIEW_DecSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//首先删除数据表格内容
					NumRows = LISTVIEW_GetNumRows(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					for(i=0; i < NumRows; i++)LISTVIEW_DeleteRow(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1),0);		
					
					//得到时间表格第几个时间点
					inx = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//得到这个时间点的开始时间
					pTabTemp = Get_TimeTab_Time(inx);
					sprintf((char *)text_buffer, "%d/%d/%d %d:%d:%d",
						(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff),	//显示开始时间值
						(pTabTemp[1]>>8), (pTabTemp[1]&0x00ff), (pTabTemp[2]>>8), (pTabTemp[2]&0x00ff) );
					TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT1),text_buffer);	
					
					Num_Temp_bf = 0;
					//这个时间点之前有多少个数据
					for(i = 0; i < inx; i++)
					{
						Num_Temp_bf += Get_TimeTab_num(i);
					}
					//这个时间点有多少个数据
					Num_Temp = Get_TimeTab_num(inx);
					for(i = 0; i < Num_Temp; i++)
					{
						//在以前的基础上获取数据并显示
						pTabTemp = Get_DataTab(i + Num_Temp_bf);
						NumToStr(pTabTemp[0],pTabTemp[1]);								
						for (r = 0; r < 2; r++)
						{
							pText[r] = acText2[r];
						}
						LISTVIEW_InsertRow(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1),i,(const GUI_ConstString *)pText);
					}
											
					LISTVIEW_IncSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));

				break;
				
				case GUI_KEY_LIST1_REFRESH_DOWN: 
					LISTVIEW_IncSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//首先删除数据表格内容
					NumRows = LISTVIEW_GetNumRows(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					for(i=0; i < NumRows; i++)LISTVIEW_DeleteRow(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1),0);		
					
					//得到时间表格第几个时间点
					inx = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//得到这个时间点的开始时间
					pTabTemp = Get_TimeTab_Time(inx);
					sprintf((char *)text_buffer, "%d/%d/%d %d:%d:%d",
						(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff),	//显示开始时间值
						(pTabTemp[1]>>8), (pTabTemp[1]&0x00ff), (pTabTemp[2]>>8), (pTabTemp[2]&0x00ff) );
					TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT1),text_buffer);	
				
					Num_Temp_bf = 0;
					//这个时间点之前有多少个数据
					for(i = 0; i < inx; i++)
					{
						Num_Temp_bf += Get_TimeTab_num(i);
					}
					//这个时间点有多少个数据
					Num_Temp = Get_TimeTab_num(inx);
					for(i = 0; i < Num_Temp; i++)
					{
						//在以前的基础上获取数据并显示
						pTabTemp = Get_DataTab(i + Num_Temp_bf);
						NumToStr(pTabTemp[0],pTabTemp[1]);								
						for (r = 0; r < 2; r++)
						{
							pText[r] = acText2[r];
						}
						LISTVIEW_InsertRow(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1),i,(const GUI_ConstString *)pText);
					}
											
					LISTVIEW_IncSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					
				break;
					
				case GUI_KEY_LIST2_REFRESH_UP: 
					LISTVIEW_DecSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					
					//得到时间表格第几个时间点
					inx = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//得到这个时间点的开始时间
					pTabTemp = Get_TimeTab_Time(inx);
					//得到数据表的位置
					t_Temp1 = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					//计算这个点的时间
					t_Temp1 = t_Temp1*SaveTimeJGTab[inx];//得到总秒数
					//最多24个小时					
					t_Temp = ((pTabTemp[1]&0x00ff)*60+(pTabTemp[2]>>8))*60+(pTabTemp[2]&0x00ff)+t_Temp1;
					
					sprintf((char *)text_buffer, "%d/%d/%d %d:%d:%d",
						(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff), 
						(pTabTemp[1]>>8), t_Temp/3600, (t_Temp%3600)/60, (t_Temp%3600)%60 ); //增加时间值					
					TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT1),text_buffer);	
					//t_Temp/3600     	//小时
					//(t_Temp%3600)/60 	//分钟
					//(t_Temp%3600)%60 	//秒钟
				
				break;
				
				case GUI_KEY_LIST2_REFRESH_DOWN: 
					LISTVIEW_IncSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					
					//得到时间表格第几个时间点
					inx = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW0));
					//得到这个时间点的开始时间
					pTabTemp = Get_TimeTab_Time(inx);
					//得到数据表的位置
					t_Temp1 = LISTVIEW_GetSel(WM_GetDialogItem(hWin, GUI_ID_LISTVIEW1));
					//计算这个点的时间
					t_Temp1 = t_Temp1*SaveTimeJGTab[inx];//得到总秒数
					//最多24个小时					
					t_Temp = ((pTabTemp[1]&0x00ff)*60+(pTabTemp[2]>>8))*60+(pTabTemp[2]&0x00ff)+t_Temp1;
					
					sprintf((char *)text_buffer, "%d/%d/%d %d:%d:%d",
						(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff), 
						(pTabTemp[1]>>8), t_Temp/3600, (t_Temp%3600)/60, (t_Temp%3600)%60 ); //增加时间值					
					TEXT_SetText(WM_GetDialogItem(hWin, GUI_ID_TEXT1),text_buffer);	
					//t_Temp/3600     	//小时
					//(t_Temp%3600)/60 	//分钟	
					//(t_Temp%3600)%60 	//秒钟
				
				break;
				
				default:
				break;
			}
		break; 
		default:		
		WM_DefaultProc(pMsg);break; 
	} //1
}
/*******************************************************************************
* 函数名称: _cbListView0()
* 功能描述: _cbListView0，控件回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbListView0(WM_MESSAGE * pMsg) 
{
	const WM_KEY_INFO * pInfo;
	switch (pMsg->MsgId) 
	{
	case WM_KEY: 
		pInfo = (WM_KEY_INFO *)pMsg->Data.p;
		switch(pInfo->Key)
		{
		case GUI_KEY_RIGHT:
			GUI_StoreKeyMsg(GUI_KEY_FOCUS_NEXT, 1);
			break;
		case GUI_KEY_UP:
			GUI_StoreKeyMsg(GUI_KEY_LIST1_REFRESH_UP, 1);
			break;
		case GUI_KEY_DOWN:
			GUI_StoreKeyMsg(GUI_KEY_LIST1_REFRESH_DOWN, 1);
			break;
		default:
			LISTVIEW_Callback(pMsg); 
			break;
		}
		break;
	default:
		LISTVIEW_Callback(pMsg); 
		break;
	}
}
/*******************************************************************************
* 函数名称: _cbListView1()
* 功能描述: _cbListView1，控件回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbListView1(WM_MESSAGE * pMsg) 
{
	const WM_KEY_INFO * pInfo;
	switch (pMsg->MsgId) 
	{
	case WM_KEY: 
		pInfo = (WM_KEY_INFO *)pMsg->Data.p;
		switch(pInfo->Key)
		{
		case GUI_KEY_LEFT:
			GUI_StoreKeyMsg(GUI_KEY_FOCUS_PREV, 1);
			break;
		case GUI_KEY_UP:
			GUI_StoreKeyMsg(GUI_KEY_LIST2_REFRESH_UP, 1);
			break;
		case GUI_KEY_DOWN:
			GUI_StoreKeyMsg(GUI_KEY_LIST2_REFRESH_DOWN, 1);
			break;
		default:
			LISTVIEW_Callback(pMsg); 
			break;
		}
		break;
	default:
		LISTVIEW_Callback(pMsg); 
		break;
	}
}

/*******************************************************************************
* 函数名称: CreatRecordWin()
* 功能描述: CreatRecordWin，创建记录界面
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void CreatRecordWin(void)
{
	uint8_t    i=0;
	uint8_t    r=0;
	const char * pText[3];
	char   acText1[2][20] = { {0} }; 
	uint16_t Num_Temp = 0;
	uint16_t *pTabTemp = 0;
	char text_buffer[10]={0};
	TEXT_Handle hText;
	LISTVIEW_Handle hListView;	
	uint16_t    RecordWinCntNum = 0;
	//建立桌面窗口的子窗口--记录窗口
	WinPara.hWinRecord = WM_CreateWindowAsChild(WinPara.xPosWin ,											
											  WinPara.yPosWin ,	//位置
											  WinPara.xSizeWin,
											  WinPara.ySizeWin,	//底部剩余宽度
											  WM_HBKWIN, WM_CF_SHOW|WM_CF_MEMDEV, _cbRecordWin, sizeof(WIN_PARA *)
											);	
	hText=TEXT_CreateEx(5, 200, 310, 20,WinPara.hWinRecord, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0,"Historical records");
	TEXT_SetTextColor(hText, GUI_BLUE);
	TEXT_SetFont(hText,GUI_FONT_20_ASCII);

	hText=TEXT_CreateEx(165, 190, 146, 10,WinPara.hWinRecord, WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT1,"2016/11/23 10:38:45");
	TEXT_SetFont(hText,GUI_FONT_16B_ASCII);
	TEXT_SetTextColor(hText, GUI_BLACK);

	pTabTemp = Get_TimeTab_Time(0);
	sprintf((char *)text_buffer, "%d/%d/%d %d:%d:%d",(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff), 
													 (pTabTemp[1]>>8),       (pTabTemp[1]&0x00ff), 
													 (pTabTemp[2]>>8),       (pTabTemp[2]&0x00ff) ); //增加时间值					
	TEXT_SetText(hText,text_buffer);												  
	//建立数据表控件
	hListView = LISTVIEW_CreateEx(10, 5,150, 195,WinPara.hWinRecord, WM_CF_SHOW,0, GUI_ID_LISTVIEW0);
	WM_SetCallback(hListView, _cbListView0);
	WIDGET_SetEffect(hListView, &WIDGET_Effect_None);
											  
	HEADER_SetDragLimit(LISTVIEW_GetHeader(hListView), 1); // 将拖动分割线限制为打开
	LISTVIEW_SetGridVis(hListView , 1);     //设置网格线可见											  
	for (i = 0; i < 2; i++) 
	{
		LISTVIEW_AddColumn(hListView , _aColRecordTime[i].Width, _aColRecordTime[i].pText, _aColRecordTime[i].Align);
		LISTVIEW_SetCompareFunc(hListView , i, _aColRecordTime[i].fpCompare);
	}	
	//计算时间点，如果有0则调整数据	
	RecordWinCntNum = 0;
	for(i = 0; i < TimeTabItemNum; i++)
	{
		if(Get_TimeTab_num(i) == 0)
		{
			MoveUp_TimeDataTab(i);
		}
		else
		{
			RecordWinCntNum ++;
		}
	}	
	for(i = 0; i < RecordWinCntNum; i++)
	{
		pTabTemp = Get_TimeTab_Time(i);
		sprintf((char *)acText1[0], "%d/%d/%d %d:%d:%d",(pTabTemp[0]>>8)+2000, (pTabTemp[0]&0x00ff), 
														(pTabTemp[1]>>8),      (pTabTemp[1]&0x00ff), 
														(pTabTemp[2]>>8),      (pTabTemp[2]&0x00ff) ); //增加时间值
		Num_Temp = Get_TimeTab_num(i);
		sprintf((char *)acText1[1], "%d",Num_Temp);
		for (r = 0; r < 2; r++) 
		{
			pText[r] = acText1[r];
		}
		LISTVIEW_AddRow(hListView, (const GUI_ConstString *)pText);
	}	
	LISTVIEW_EnableSort(hListView); //启用排序功能
	SCROLLBAR_CreateAttached(hListView,SCROLLBAR_CF_VERTICAL);
	LISTVIEW_SetSel(hListView,0);	
	WM_SetFocus(hListView);	
//*******************************************************************************************************************************	
	hListView = LISTVIEW_CreateEx(165, 5,146, 183,WinPara.hWinRecord, WM_CF_SHOW,0, GUI_ID_LISTVIEW1);
	WM_SetCallback(hListView, _cbListView1);
	WIDGET_SetEffect(hListView, &WIDGET_Effect_None);
	LISTVIEW_SetGridVis(hListView , 1);     //设置网格线可见
//	LISTVIEW_SetAutoScrollV(hListView , 1); 											  
	for (i = 0; i < 2; i++) 
	{
		LISTVIEW_AddColumn(hListView , _aColRecordData[i].Width, _aColRecordData[i].pText, _aColRecordData[i].Align);
		LISTVIEW_SetCompareFunc(hListView , i, _aColRecordData[i].fpCompare);
	}
	Num_Temp = Get_TimeTab_num(0);
	for(i = 0; i < Num_Temp; i++)
	{
		pTabTemp = Get_DataTab(i);
		NumToStr(pTabTemp[0],pTabTemp[1]);
		for (r = 0; r < 2; r++) 
		{
			pText[r] = acText2[r];
		}
		LISTVIEW_AddRow(hListView, (const GUI_ConstString *)pText);
	}
	LISTVIEW_EnableSort(hListView); //启用排序功能
	SCROLLBAR_CreateAttached(hListView,SCROLLBAR_CF_VERTICAL);
	LISTVIEW_SetSel(hListView,0);	
	GUI_Exec();	
}



/*******************************************************************************
* 函数名称: CreatPageStartWin()
* 功能描述: CreatPageStartWin 开机画面函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
void CreatPageStartWin(void)
{
	uint16_t i = 0;
	if(LogoOnOff==1)
    {
		WM_SetDesktopColor(GUI_WHITE);	
		GUI_Exec();	
		GUI_DrawBitmap(&bmGBLOGO,66,36);//绘制logo
		bsp_delay_ms(1);
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);//背光
		bsp_delay_ms(1600);
    }
	else
    {
		WM_SetDesktopColor(GUI_BLACK);	
		GUI_Exec();	
		GUI_SetPenSize(5);	
		GUI_SetColor(0x70D869);
		bsp_delay_ms(1);
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);//背光
		for(i=0;i<100;i++)
		{
			GUI_DrawPoint(i, 120);	
			bsp_delay_ms(3);
		}
		for(i=100;i<130;i++)
		{
			GUI_DrawPoint(i, 120-i+100);	
			bsp_delay_ms(3);	
		}
		for(i=130;i<180;i++)
		{
			GUI_DrawPoint(i, i-40);	
			bsp_delay_ms(3);
		}
		for(i=180;i<200;i++)
		{
			GUI_DrawPoint(i, 140-i+180);	
			bsp_delay_ms(3);
		}
		for(i=200;i<320;i++)
		{
			GUI_DrawPoint(i, 120);	
			bsp_delay_ms(3);
		}	
	}
}
/*******************************************************************************
* 函数名称: _cbBKWin()
* 功能描述: _cbBKWin,桌面背景的回调函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
static void _cbBKWin(WM_MESSAGE * pMsg)
{  
	switch (pMsg->MsgId) 
	{
	 case WM_PAINT:			//重绘背景																
		GUI_SetColor(GUI_DARKGRAY); //设置为深灰色
		GUI_FillRect(0,0,LCD_GetXSize(),LCD_GetYSize()); //填充矩形
	  break;
	 default:		 
	 WM_DefaultProc(pMsg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: _cbTimer
*	功能说明: 定时器回调函数		
*	形    参: pTM 消息指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbTimer(GUI_TIMER_MESSAGE * pTM)
{
	switch (pTM->Context) 
	{
		case 0x01://20ms
			GUI_KeyTask();//定时20ms扫描按键
			/* 此函数一定要调用，设置重新启动，要不仅执行一次 */
			GUI_TIMER_Restart(pTM->hTimer);
			break;
		case 0x02://250ms
			if(WM_IsWindow(WinPara.hWinStatus))//判断窗口是否有效
			{
				WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_RTC); //更新RTC
				WM_SendMessageNoPara(WinPara.hWinStatus,MY_MSG_GAUTO);//“ATUTO”显示
			}
			/* 此函数一定要调用，设置重新启动，要不仅执行一次 */
			GUI_TIMER_Restart(pTM->hTimer);
			break;
		default:
			break;
	}
}

/*******************************************************************************
* 函数名称: MainTask()
* 功能描述: MainTask公有函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/
void MainTask(void) 
{	
	GUI_TIMER_HANDLE hTimer;
	//准备建立窗口，以下是使用到的用户定义参数，方便在回调函数中使用
	WinPara.xSizeLCD = LCD_GetXSize();						//LCD屏幕尺寸--320
	WinPara.ySizeLCD = LCD_GetYSize();						//LCD屏幕尺寸--240
	WinPara.xSizeWin = WinPara.xSizeLCD;					//窗口尺寸
	WinPara.ySizeWin = WinPara.ySizeLCD - 20;		  		//窗口尺寸 屏幕大小减去状态栏和控制栏
	WinPara.xPosWin	 = 0;									//窗口的起始位置
	WinPara.yPosWin  = 20;									//窗口的起始位置
	WinPara.xSizeWinVirtual = WinPara.xSizeWin * VIRTUAL_WIN_NUM;	//虚拟窗口尺寸，用于桌面
	WM_EnableMemdev(WM_HBKWIN);
	GUI_UC_SetEncodeUTF8(); 
	
	/* 设置背景桌面的回调函数 */
	WM_SetCallback(WM_HBKWIN, _cbBKWin);	
	
	/* 创建定时器 */
	hTimer = GUI_TIMER_Create(_cbTimer, /* 回调函数 */
							  20,        /* 绝对时间，设置系统上电后1ms作为溢出时间 */
							  0x01,     /* 可以认为此参数是区分不同定时器的ID，方便多个定时使用同一个回调函数 */ 
							  0);       /* 保留，暂时未用到 */
	GUI_TIMER_SetPeriod(hTimer, 20);
	
	/* 创建定时器 */
	hTimer = GUI_TIMER_Create(_cbTimer, /* 回调函数 */
							  250,        /* 绝对时间，设置系统上电后1ms作为溢出时间 */
							  0x02,     /* 可以认为此参数是区分不同定时器的ID，方便多个定时使用同一个回调函数 */ 
							  0);       /* 保留，暂时未用到 */
	GUI_TIMER_SetPeriod(hTimer, 250);
	/* 创建状态栏、一页面*/
	CreatStatusWin();
	CreatOneWin();	
	
}

/*******************************************************************************
* 函数名称: GUI_Power_Off()
* 功能描述: GUI_Power_Off,GUI关机调用函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/

void GUI_Power_Off(void)
{
	bsp_WriteCpuFlash(SAVE_ADDR_Page3_data_flash, page3_data_flash, 13*5);
	bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJGTab, SaveTimeJGTab, TimeTabItemNum);
}
/*******************************************************************************
* 函数名称: GUI_Power_On()
* 功能描述: GUI_Power_On,GUI开机调用函数
* 输入参数: void
* 返回参数: 无
********************************************************************************/

void GUI_Power_On(void)
{
	int i = 0;
	bsp_ReadCpuFlash_HalfWord(SAVE_ADDR_Page3_data_flash,page3_data_flash,13*5);  //读取gui 时间数据表
	if((page3_data_flash[0]==0xffff)||(page3_data_flash[13]==0xffff)||(page3_data_flash[26]==0xffff)||(page3_data_flash[52]==0xffff))
	{
		for(i=0;i<(13*5);i++)
		{
			page3_data_flash[i] = 0;
		}
		bsp_WriteCpuFlash(SAVE_ADDR_Page3_data_flash, page3_data_flash, 13*5);
	}
	bsp_ReadCpuFlash_HalfWord(SAVE_ADDR_SaveTimeJGTab,SaveTimeJGTab,TimeTabItemNum);  
	if((SaveTimeJGTab[0]==0xffff)||(SaveTimeJGTab[1]==0xffff)||(SaveTimeJGTab[2]==0xffff)||(SaveTimeJGTab[3]==0xffff))
	{
		for(i=0;i<TimeTabItemNum;i++)
		{
			SaveTimeJGTab[i] = 3;
		}
		bsp_WriteCpuFlash(SAVE_ADDR_SaveTimeJGTab, SaveTimeJGTab, TimeTabItemNum);
	}
}
