
#ifndef _GEARSSWITCH_H_
#define _GEARSSWITCH_H_

void bsp_Relay_Init(void);
uint16_t DetectGear(void);
void SetGear(uint8_t GValue);
uint16_t RefreshGear(void);



#endif

