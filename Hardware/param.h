/*
*********************************************************************************************************
*
*	模块名称 : 应用程序参数模块
*	文件名称 : param.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __PARAM_H
#define __PARAM_H

#define PARAM_SAVE_TO_FLASH		/* 参数存储到CPU内部Flash */

#ifdef PARAM_SAVE_TO_FLASH
	#define PARAM_ADDR	 ADDR_FLASH_SECTOR_11		/* 0x080E0000 Flash最后128K扇区用来存放参数 */
#endif
#define PARAM_VER			0x00000104					/* 参数版本 */

/* 全局参数 */
typedef struct
{
	uint32_t ParamVer;			/* 参数区版本控制（可用于程序升级时，决定是否对参数区进行升级） */


//	/* uip ip 地址参数 */
//	uint8_t uip_ip[4];			/* 本机IP地址 */
//	uint8_t uip_net_mask[4];	/* 子网掩码 */
//	uint8_t uip_gateway[4];	/* 默认网关 */

//	/* lwip ip 地址参数 */
//	uint8_t lwip_ip[4];			/* 本机IP地址 */
//	uint8_t lwip_net_mask[4];	/* 子网掩码 */
//	uint8_t lwip_gateway[4];	/* 默认网关 */

//	/* 收音机参数 */
//	uint8_t ucRadioMode;		/* AM 或 FM */
//	uint8_t ucRadioListType;		/* 电台列表类型。武汉地区或全国 */
//	uint8_t ucIndexFM;			/* 当前FM电台索引 */
//	uint8_t ucIndexAM;			/* 当前电台索引 */
//	uint8_t ucRadioVolume;		/* 音量 */
//	uint8_t ucSpkOutEn;			/* 扬声器输出使能 */
//	
//	uint8_t Addr485;
//	uint32_t Baud485;
}
PARAM_T;

extern PARAM_T g_tParam;

void LoadParam(void);
void SaveParam(void);

#endif
