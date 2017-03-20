#ifndef __INTER_FLASH_H
#define __INTER_FLASH_H

#define FLASH_BASE_ADDR	0x08000000		/* Flash基地址 */
#define	FLASH_SIZE		(512*1024)		/* Flash 容量 */

/* 对于F103，  512K FLASH , 每个PAGE = 2K 字节，总共 256个 PAGE  */
#define SECTOR_MASK			0xFFFFF800

/* FLASH地址 - PAGE SIZE = 2K字节 */
#define SAVE_ADDR(x)		(FLASH_BASE_ADDR + 0x80000 - (x*0x800))

#define SAVE_ADDR_k_value          SAVE_ADDR(1)
#define SAVE_ADDR_SaveGearsValue   SAVE_ADDR(2)
#define SAVE_ADDR_ConectFrqcyOnOff SAVE_ADDR(3)
#define SAVE_ADDR_SaveTimeJG       SAVE_ADDR(4)
#define SAVE_ADDR_LogoOnOff        SAVE_ADDR(5)
#define SAVE_ADDR_PowrOffTime      SAVE_ADDR(6)
#define SAVE_ADDR_Page3_data_flash SAVE_ADDR(7)
#define SAVE_ADDR_RangeMAX         SAVE_ADDR(8)

#define SAVE_ADDR_SaveTimeCnt      SAVE_ADDR(9)
#define SAVE_ADDR_Save_DataTab     SAVE_ADDR(10)
#define SAVE_ADDR_SaveTimeJGTab    SAVE_ADDR(11)



#define FLASH_IS_EQU		0   /* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */

uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize);
uint8_t bsp_ReadCpuFlash_HalfWord(uint32_t _ulFlashAddr,uint16_t *_ucpDst,uint32_t _ulSize);
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint16_t *_ucpSrc, uint32_t _ulSize);
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint16_t *_ucpBuf, uint32_t _ulSize);

void Read_Stored_Data(void);

#endif
