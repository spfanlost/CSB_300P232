#ifndef __AD_H
#define __AD_H 			   

void ADC1_Channel_1_init(void);
void ADC2_Channel_2_init(void);
void ADC2_Channel_3_init(void);
void ADC2_Channel_10_init(void);

uint16_t ADC1_Channel_1_get(void);//半波测频               
uint16_t ADC2_Channel_2_get(void);//声强测量
uint16_t ADC2_Channel_3_get(void);//全波测频
uint16_t ADC2_Channel_10_get(void);//电池电压测量
uint16_t Get_Bat_Average(void);//电池电压均值测量

#endif

