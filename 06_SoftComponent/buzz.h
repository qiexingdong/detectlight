#ifndef  BUZZ_H       
#define  BUZZ_H

#include "gpio.h"
 
#define Buzz (PTA_NUM|8) //37������

void Buzz_Init(void);
void Buzz_On(void);
void Buzz_Off(void);

#endif