#ifndef _tim_H_
#define _tim_H_
#include "stm32f10x.h"



#define TIME4_Preiod  1000

void TIM4_Init(char clock,int Preiod);//用于监测系统
void TIM3_Init(char clock,int Preiod);//定时器3的初始化
void TimerNVIC_Configuration(void);//定时器中断向量表配置

extern u16 flag10Hz,flag50Hz,flag80Hz,flag100Hz;


#endif

