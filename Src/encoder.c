#include "encoder.h"
#include <stdint.h>

uint8_t	cnt = 0;  // Текущее значение счётчика энкодера (0..9)
/**
  * @brief  Инициализация таймера 3 для работы с энкодером
  * @param  None
  * @retval None
  */
void initEncoderTIM3(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; 		// Включаем тактирование порта A (reset-clock control -> шина APB2 -> enable port GPIOA)
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; 		// Включаем тактирование таймера 3 (шина APB1)

    GPIOA->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7);   //Очистка битов CNF6 и CNF7 MODE6 и MODE7
    GPIOA->CRL |= (GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1);                                  //Выставить 10 (вторые биты в 1) 
                                                                    
    TIM3->ARR = 40;       // Модуль счета (auto-reload) 36. Энкодер делает 2 импульса на 1 значение на 7-сегм + запас для ограничений
    TIM3->CNT = 12;       // Начальное значение счётчика 18. Запас для ограничений
}
/**
  * @brief  Получение данных с энкодера и ограничение значений
  * @param  None
  * @retval None
  */
void getEncoderData(void)
{
	LIMIT_UP_CNT();                          //Макрос ограничения сверху (если больше 36, то ставим 36)
	LIMIT_DOWN_CNT();                        //Макрос ограничения снизу (если меньше 18, то ставим 18)
	cnt = (uint8_t)((TIM3->CNT - 18) / 2);   //Преобразование значения счетчика в диапазон 0..9 
}
/**
  * @brief  Получение текущего значения счётчика энкодера
  * @param  None
  * @retval Текущее значение счётчика
  */
uint8_t getCntValue(void)
{
    return cnt;   //Возврат текущего значения счетчика (передать глобальную переменную в main.c)
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
    uint8_t val = getCntValue();  // Копируем во временную переменную (0..9) 

    // Преобразуем его в частоту или период (чем больше val — тем выше частота)
    // Например: базовый ARR = 1000, диапазон 100–2000, шаг 200
    uint16_t newARR = 2000 - (val * 200);   // уменьшение ARR повышает частоту
    if (newARR < 100) newARR = 100;         //ограничение снизу
    if (newARR > 2000) newARR = 2000;       //ограничение сверху

    //Оптимизация: обновляем только если значение изменилось
    static uint16_t lastARR = 0;  //static - можно брать из кэша функции
    if (newARR == lastARR) {
        return; // Значение не изменилось, выходим
    }
    lastARR = newARR;

    //будем обновлять регистры, надо вручную выставить предделитель и ARR
    TIM2->PSC = 64000 - 1;       // Предделитель остаётся постоянным (64МГц / 64000 = 1кГц), он сам +1 для предотвращения деления на ноль
    TIM2->ARR = newARR;          // Обновляем Auto-Reload Register новым значением
    TIM2->EGR |= TIM_EGR_UG;     // Немедленное обновление регистров (Update event generation). Бит очищается аппаратно 

}
