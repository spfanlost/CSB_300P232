#ifndef __key
#define __key 

#define KEY_COUNT    7	   					/* 按键个数 */

/* 根据应用程序的功能重命名按键宏 */
#define KEY_DOWN_K1		KEY_1_DOWN		/*ESC*/
#define KEY_UP_K1			KEY_1_UP
#define KEY_LONG_K1		KEY_1_LONG

#define KEY_DOWN_K2		KEY_2_DOWN		/*OK*/
#define KEY_UP_K2			KEY_2_UP
#define KEY_LONG_K2		KEY_2_LONG

#define JOY_DOWN_U		KEY_3_DOWN		/* 上 */
#define JOY_UP_U			KEY_3_UP
#define JOY_LONG_U		KEY_3_LONG

#define JOY_DOWN_D		KEY_4_DOWN		/* 下 */
#define JOY_UP_D			KEY_4_UP
#define JOY_LONG_D		KEY_4_LONG

#define JOY_DOWN_L		KEY_5_DOWN		/* 左 */
#define JOY_UP_L			KEY_5_UP
#define JOY_LONG_L		KEY_5_LONG

#define JOY_DOWN_R		KEY_6_DOWN		/* 右 */
#define JOY_UP_R			KEY_6_UP
#define JOY_LONG_R		KEY_6_LONG


#define KEY_DOWN_K1D	KEY_9_DOWN		/* K1 K2 组合键 */
#define KEY_UP_K1D	    KEY_9_UP
#define KEY_LONG_K1D	KEY_9_LONG

/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
typedef enum
{
	KID_ESC = 0,
	KID_OK,
	KID_U,
	KID_D,
	KID_L,
	KID_R,
}KEY_ID_E;

/*
	按键滤波时间50ms, 单位10ms。
	只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
	即使按键电路不做硬件滤波，该滤波机制也可以保证可靠地检测到按键事件
*/
#define KEY_FILTER_TIME   5
#define KEY_LONG_TIME     100			/* 单位20ms， 持续 秒，认为长按事件 */

/*
	定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件

	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2) 编译器可帮我们避免键值重复。
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 表示按键事件 */

	KEY_1_DOWN,				/* 1键按下 */
	KEY_1_UP,					/* 1键弹起 */
	KEY_1_LONG,				/* 1键长按 */

	KEY_2_DOWN,				/* 2键按下 */
	KEY_2_UP,					/* 2键弹起 */
	KEY_2_LONG,				/* 2键长按 */

	KEY_3_DOWN,				/* 3键按下 */
	KEY_3_UP,					/* 3键弹起 */
	KEY_3_LONG,				/* 3键长按 */

	KEY_4_DOWN,				/* 4键按下 */
	KEY_4_UP,					/* 4键弹起 */
	KEY_4_LONG,				/* 4键长按 */

	KEY_5_DOWN,				/* 5键按下 */
	KEY_5_UP,					/* 5键弹起 */
	KEY_5_LONG,				/* 5键长按 */

	KEY_6_DOWN,				/* 6键按下 */
	KEY_6_UP,					/* 6键弹起 */
	KEY_6_LONG,				/* 6键长按 */

	/* 组合键 */
	KEY_9_DOWN,				/* 9键按下 */
	KEY_9_UP,				/* 9键弹起 */
	KEY_9_LONG,				/* 9键长按 */

}KEY_ENUM;

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t  Count;			/* 滤波器计数器 */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t  RepeatSpeed;	/* 连续按键周期 */
	uint8_t  RepeatCount;	/* 连续按键计数器 */
}KEY_T;

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	6
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针1 */
	uint8_t Write;					/* 缓冲区写指针 */
	uint8_t Read2;					/* 缓冲区读指针2 */
}KEY_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_Init_Key(void);
void bsp_KeyScan(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
uint8_t bsp_GetKey2(void);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed);
void bsp_ClearKey(void);

void bsp_Power_init(void);
void bsp_OnOffKey_init(void);
void GUI_KeyTask(void);
void BacklightOn(void);
void BacklightOff(void);

#endif
