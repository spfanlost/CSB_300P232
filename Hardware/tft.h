#ifndef __TFT_H
#define __TFT_H		
#include "stm32f10x.h"                  // Device header

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F//洋红色
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF//青色
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色

#define R32            0x20
#define R33            0x21
#define R34            0x22


//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

#define	LCD_CS_SET  GPIOE->BSRR=1<<11    //片选端口  		
#define	LCD_RS_SET	GPIOE->BSRR=1<<12    //数据/命令 		 
#define	LCD_WR_SET	GPIOE->BSRR=1<<13    //写数据			
#define	LCD_RD_SET	GPIOE->BSRR=1<<14    //读数据			
								    
#define	LCD_CS_CLR  GPIOE->BRR=1<<11     //片选端口  		
#define	LCD_RS_CLR	GPIOE->BRR=1<<12     //数据/命令		  
#define	LCD_WR_CLR	GPIOE->BRR=1<<13     //写数据			
#define	LCD_RD_CLR	GPIOE->BRR=1<<14     //读数据			  	

//PB0~15,作为数据线
#define DATAOUT(x) GPIOB->ODR=x; //数据输出
#define DATAIN     GPIOB->IDR;   //数据输入		 

//LCD重要参数集
typedef struct  
{										    
	uint16_t width;			//LCD 宽度
	uint16_t height;		//LCD 高度
	uint16_t id;			//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标指令
	uint16_t setycmd;		//设置y坐标指令	 
}_lcd_dev; 	  

extern _lcd_dev lcddev;    
//LCD的画笔颜色和背景色	   
extern uint16_t  POINT_COLOR;//默认红色    
extern uint16_t  BACK_COLOR; //背景颜色.默认为黑色

void TFT_Init(void);	
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue);
void LCD_WR_REG(uint16_t data);
void LCD_WR_DATA(uint16_t data);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
uint16_t LCD_RD_DATA(void);
uint16_t LCD_RD_REG_DATA(void);
void LCD_WriteRAM_Prepare(void);
void LCD_Display_Dir(uint8_t dir);
void LCD_Scan_Dir(uint8_t dir);

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_Clear(uint16_t color);
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y);

//void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);						//显示一个字符
//void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);  						//显示一个数字
//void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//显示 数字
//void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p);		//显示一个字符串,12/16字体

#endif
