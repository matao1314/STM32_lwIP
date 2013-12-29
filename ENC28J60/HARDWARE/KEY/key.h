#ifndef __KEY_H
#define	__KEY_H

#include "stm32f10x.h"
#define KEY_UP 		1
#define KEY_DOWN	0

void KEY_Init(void);
u8   KEY_Scan(void);

#endif 

