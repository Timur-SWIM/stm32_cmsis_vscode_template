#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "stm32f1xx.h"
#include "global.h"
#include "uart.h"
#include <stdbool.h>

#define SET_LED_NUM(led_num) GPIOC->ODR = GPIOC->ODR == 0 ? 0 : led_num
#define LED_NUM_SWAP(led_num)	GPIOC->ODR = GPIOC->ODR == 0 ? led_num : 0

void setDisplay(uint8_t cnt);
void initLED(void);
void init_button(void);
void initTIM2(void);
void updateDisplayIfChanged(uint8_t newValue);

#endif
