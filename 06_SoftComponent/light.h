#ifndef  LIGHT_H       
#define  LIGHT_H

#include "gpio.h"

#define Light_1 (PTC_NUM|8) //3������ �������������
#define Light_2 (PTC_NUM|7) //4������ ��������������

void Light_Init(void);
void Light_1_On(void);
void Light_1_Off(void);
void Light_2_On(void);
void Light_2_Off(void);

#endif


