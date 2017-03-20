#ifndef _SAVE_H
#define _SAVE_H


void Add_TimeTab_time(uint8_t tcnt);
void Add_TimeTab_Dest(uint8_t tcnt,uint16_t Dest);
void Add_TimeTab_num(uint8_t tcnt, uint16_t num);
void Clear_TimeTab(uint8_t tcnt);
uint16_t Get_TimeTab_num(uint8_t tcnt);
uint16_t* Get_TimeTab_Time(uint8_t tcnt);
uint16_t Get_TimeTab_Dest(uint8_t tcnt);
void Add_DataTab(uint16_t dat);
uint16_t* Get_DataTab(uint16_t dat);
void Clear_DataTab(uint16_t dat);
uint8_t TimeTabHaveData(uint8_t tcnt);
uint8_t DataTabHaveData(uint16_t dat);
void Clear_TimeDataTab(uint16_t n);
void MoveUp_TimeDataTab(uint16_t n);
void SaveTabRead(void);
void PowerOffSave(void);

#endif



