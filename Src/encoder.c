#include "encoder.h"
#include <stdint.h>

uint8_t	cnt = 0;
/**
  * @brief  Инициализация таймера 3 для работы с энкодером
  * @param  None
  * @retval None
  */
void initEncoderTIM3(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; 		// Enable GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; 		// Enable TIM3 clock
    // Configure Pins 6 and 7 of Port A as alternate function inputs
    GPIOA->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1);
	// Configure TIM3 for encoder interface                                                                     
    TIM3->ARR = 40;
    TIM3->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0; // Capture on TI1 and TI2
    TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);    // Rising edge polarity
    TIM3->SMCR = TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;      // Enable encoder mode 3 (Counts on both TI1 and TI2 edges)
    TIM3->CR1 |= TIM_CR1_CEN;                          // Enable timer
    TIM3->CCMR1 |= (TIM_CCMR1_IC1F_3 | TIM_CCMR1_IC1F_2 | TIM_CCMR1_IC1F_0);
    TIM3->CCMR1 |= (TIM_CCMR1_IC2F_3 | TIM_CCMR1_IC2F_2 | TIM_CCMR1_IC2F_0);
    TIM3->CNT = 12;                   // Reset cnt
}
/**
  * @brief  Получение данных с энкодера и ограничение значений
  * @param  None
  * @retval None
  */
void getEncoderData(void)
{
	LIMIT_UP_CNT();
	LIMIT_DOWN_CNT();
	cnt = (uint8_t)((TIM3->CNT - 18) / 2);
}
/**
  * @brief  Получение текущего значения счётчика энкодера
  * @param  None
  * @retval Текущее значение счётчика
  */
uint8_t getCntValue(void)
{
    return cnt;
}
/**
  * @brief  Обновление частоты таймера 2 на основе значения энкодера
  * @param  None
  * @retval None
  */
void updateTIM2Freq(void)
{
    // Получаем обновлённое значение cnt из энкодера
    //getEncoderData();
    uint8_t val = getCntValue();  // 0..9 (если лимиты 18..36 и деление на 2)

    // Преобразуем его в частоту или период (чем больше val — тем выше частота)
    // Например: базовый ARR = 1000, диапазон 100–2000
    uint16_t newARR = 2000 - (val * 200); // уменьшение ARR повышает частоту
    if (newARR < 100) newARR = 100;
    if (newARR > 2000) newARR = 2000;

    static uint16_t lastARR = 0;
    if (newARR == lastARR) {
        return; // Значение не изменилось, выходим
    }
    lastARR = newARR;

    TIM2->PSC = 64000 - 1;
    TIM2->ARR = newARR;
    TIM2->EGR |= TIM_EGR_UG; // Обновляем регистры таймера новыми значениями

}
