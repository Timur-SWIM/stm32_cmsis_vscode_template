#ifndef ENCODER_H_
#define ENCODER_H_

#include "stm32f1xx.h"

#define LIMIT_UP_CNT()  (TIM3->CNT = (TIM3->CNT > 36 ? 36 : TIM3->CNT))
#define LIMIT_DOWN_CNT() (TIM3->CNT = (TIM3->CNT < 18 ? 18 : TIM3->CNT))

void initEncoderTIM3(void);
void getEncoderData(void);
uint8_t getCntValue(void);

#endif /* ENCODER_H_ */
