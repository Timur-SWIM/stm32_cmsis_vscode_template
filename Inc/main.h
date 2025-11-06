
#ifndef MAIN_H_
#define MAIN_H_
#include "stm32f1xx.h"

#define SET_LED_NUM(digit) GPIOC->ODR = GPIOC->ODR == 0 ? 0 : digit
#define LED_NUM_SWAP(digit)	GPIOC->ODR = GPIOC->ODR == 0 ? digit : 0;

#define LIMIT_UP_CNT()  (TIM3->CNT = (TIM3->CNT > 72 ? 72 : TIM3->CNT))
#define LIMIT_DOWN_CNT() (TIM3->CNT = (TIM3->CNT < 36 ? 36 : TIM3->CNT))
void delay(uint32_t delay_value);

#endif /* MAIN_H_ */
