/*********************************Copyright (c)*********************************                                      
  *                                              
  ******************************************************************************
  * 文件名称：GUI_Common.H
  * 描    述：GUI_Common头文件
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

#ifndef  __GUI_APP_H__ 
#define  __GUI_APP_H__
/**********************头文件调用及变量定义声明****************************************/
/*  包含必要的头文件  */
#include "GUI.h"  
#include "DIALOG.h"
//#include "TIMER.H" 
  

/* 用户定义的窗口消息 */
#define MY_MSG_RTC 	      (WM_USER+0)
#define MY_MSG_RTC_SETUP  (WM_USER+1)
#define MY_MSG_ADD_GDATA  (WM_USER+2)
#define MY_MSG_BATTERY 	  (WM_USER+3)
#define MY_MSG_FREQUENCY  (WM_USER+4)
#define MY_MSG_ULTRASONIC (WM_USER+5)
#define MY_MSG_ADD_DELETE (WM_USER+6)
#define MY_MSG_SAVE_FLAG  (WM_USER+7)
#define MY_MSG_GAUTO	  (WM_USER+8)
#define MY_MSG_P3_STOP    (WM_USER+9)

//#define ID_TEXT_0  (GUI_ID_USER + 0x01)
//#define ID_TEXT_1  (GUI_ID_USER + 0x02)

#define GUI_KEY_UP_LONG            35
#define GUI_KEY_ENTER_LONG         36
#define GUI_KEY_ESC_D_LONG         37
#define GUI_KEY_FOCUS_NEXT    	   38
#define GUI_KEY_FOCUS_PREV    	   39
#define GUI_KEY_LIST1_REFRESH_UP   40
#define GUI_KEY_LIST1_REFRESH_DOWN 41
#define GUI_KEY_LIST2_REFRESH_UP   42
#define GUI_KEY_LIST2_REFRESH_DOWN 43
//#define GUI_KEY_BUF_L              34
//#define GUI_KEY_BUF_R              45

#define VIRTUAL_WIN_NUM	        (1) //用于桌面，这里没有使用图标，采用单页



/* 类型定义 typedef-----------------------------------------------------------*/

typedef struct WIN_PARA{			//窗口使用到的用户定义参数，方便在回调函数中使用
	int xSizeLCD, ySizeLCD;			//LCD屏幕尺寸
	int xPosWin, yPosWin;				//窗口的起始位置
	int xSizeWin, ySizeWin;			//窗口尺寸
	int xSizeWinVirtual;				//虚拟窗口尺寸	
	
	WM_HWIN hWinStatus;		//状态栏句柄（顶部）
	WM_HWIN hWinOne;		//界面一句柄（显示）
	WM_HWIN hWinTwo;		//界面二句柄（显示）
	WM_HWIN hWinThree;		//界面三句柄（显示）
	WM_HWIN hWinRecord;		
	WM_HWIN hWinSet;		//窗口句柄
	WM_HWIN hWinSetTime;	//时间设置窗口句柄
	WM_HWIN hWinCtrl1;		//控制窗口一的句柄
	WM_HWIN hWinCtrl2;      //控制窗口二窗口
	WM_HWIN hWinAdmin;      //厂商设置窗口
//	WM_HWIN hWinConnect;    //联机窗口
}WIN_PARA;

typedef struct {
	char  *pText;
	int    Width;
	int    Align;
	int    (*fpCompare)(const void *p0, const void *p1);
} COL_PROP;

extern WIN_PARA WinPara;			//用户使用的窗口额外数据

extern GUI_CONST_STORAGE GUI_BITMAP bmGBLOGO;
extern GUI_CONST_STORAGE GUI_BITMAP bmConf1;
extern GUI_CONST_STORAGE GUI_BITMAP bmConf2;
extern GUI_CONST_STORAGE GUI_BITMAP bmClock;
extern GUI_CONST_STORAGE GUI_BITMAP bmConnect;

extern GUI_CONST_STORAGE GUI_BITMAP bmkval;
extern GUI_CONST_STORAGE GUI_BITMAP bmhval;
extern GUI_CONST_STORAGE GUI_BITMAP bmgear;

extern GUI_CONST_STORAGE GUI_BITMAP bmFREQ;
extern GUI_CONST_STORAGE GUI_BITMAP bmOFFICO;
extern GUI_CONST_STORAGE GUI_BITMAP bmSAVEICO;

//extern GUI_CONST_STORAGE GUI_BITMAP bmADDRESS;



/*  子函数定义        */
void CreatPageStartWin(void);
void MainTask(void);
void GUI_Power_On(void);
void GUI_Power_Off(void);


#endif  

