#ifndef __DELAY_H
#define __DELAY_H 			   

////////////////////////////////////////////////////////////////////////////////// 
void bsp_delay_init(void);
void bsp_delay_ms(uint32_t n);
void bsp_delay_us(uint32_t n);

uint32_t bsp_GetRunTime(void);

#endif





























