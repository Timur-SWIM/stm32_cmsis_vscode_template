#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f103xb.h"

#endif /* MAIN_H_ */

void delay(uint32_t takts);

#define LED_SWAP()		(GPIOA->ODR ^= GPIO_ODR_ODR5);
