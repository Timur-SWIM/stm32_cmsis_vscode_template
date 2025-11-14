#include "display.h"
#include "main.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char led_num = 0x003F; //Значения сегментов (вкл/выкл). Изначально отображаем 0

/**
  * @brief  Установка числа на семисегментный индикатор
  * @param  cnt - число для отображения (0-9)
  * @retval None
  */
void setDisplay(uint8_t cnt)
{
	switch (cnt)
	{
	case 0:
		led_num = LED_0;
		break;
	case 1:
		led_num = LED_1;
		break;
	case 2:
		led_num = LED_2;
		break;
	case 3:
		led_num = LED_3;
		break;
	case 4:
		led_num = LED_4;
		break;
	case 5:
		led_num = LED_5;
		break;
	case 6:
		led_num = LED_6;
		break;
	case 7:
		led_num = LED_7;
		break;
	case 8:
		led_num = LED_8;
		break;
	case 9:
		led_num = LED_9;
		break;
	}

	SET_LED_NUM(led_num);	//Вывод на 7-сегм (макрос: если было 0, оставляем 0, иначе выводим led_num)
}
/**
  * @brief  Инициализация светодиодов на порту C
  * @param  None
  * @retval None
  */
void initLED(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;		//Включаем тактирование порта C
	GPIOC->CRL &=~GPIO_CRL_CNF0;		//CNF=00 push-pull для 0 пина
	GPIOC->CRL |= GPIO_CRL_MODE0;		//MODE=11, Output 50 MHZ для 0 пина
	GPIOC->CRL &=~GPIO_CRL_CNF1;		//CNF=00 push-pull для 1 пина
	GPIOC->CRL |= GPIO_CRL_MODE1;		//MODE=11, Output 50 MHZ для 1 пина
	GPIOC->CRL &=~GPIO_CRL_CNF2;		//CNF=00 push-pull для 2 пина
	GPIOC->CRL |= GPIO_CRL_MODE2;		//MODE=11, Output 50 MHZ для 2 пина
	GPIOC->CRL &=~GPIO_CRL_CNF3;		//CNF=00 push-pull для 3 пина
	GPIOC->CRL |= GPIO_CRL_MODE3;		//MODE=11, Output 50 MHZ для 3 пина
	GPIOC->CRL &=~GPIO_CRL_CNF4;		//CNF=00 push-pull для 4 пина
	GPIOC->CRL |= GPIO_CRL_MODE4;		//MODE=11, Output 50 MHZ для 4 пина
	GPIOC->CRL &=~GPIO_CRL_CNF5;		//CNF=00 push-pull для 5 пина
	GPIOC->CRL |= GPIO_CRL_MODE5;		//MODE=11, Output 50 MHZ для 5 пина
	GPIOC->CRL &=~GPIO_CRL_CNF6;		//CNF=00 push-pull для 6 пина
	GPIOC->CRL |= GPIO_CRL_MODE6;		//MODE=11, Output 50 MHZ для 6 пина
}
/**
  * @brief  Инициализация кнопки на порту C
  * @param  None
  * @retval None
  */
void init_button(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;	/*	Включаем тактирование порта C, настройка альтернативных ф-ий (для внешнего прерывания)	*/
	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI13_PC;					/*	Кнопка на PC13 (в 4 наборе портов: PC12-PC15), поэтому настраиваем AFIO_EXTI4	*/
	EXTI->IMR |= EXTI_IMR_MR13;									/*	Interrupt mask register. Разрешение	EXTI13 генерировать, NVIC ловить*/
	EXTI->FTSR |= EXTI_FTSR_TR13;								/* Прерывание, когда кнопка нажимается (переход на уровень 0 - спад). RTSR - прерывание по нарастанию, FTSR - прерывание по спаду*/

	NVIC_EnableIRQ(EXTI15_10_IRQn);								/*Разрешили обработку прерываний с линий с 10 по 15*/
	NVIC_SetPriority(EXTI15_10_IRQn, 1);						/*Выставили прерыванию почти самый большой приоритет*/
}
/**
  * @brief  Обработчик прерывания по кнопке на PC13
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)							/*Функция для обработки прерываний, вызывается автоматически*/
{
	if ((EXTI->PR & EXTI_PR_PR13) != 0)					/*PR - pending регистр, каждый бит которого соотв. флагу прерывания на опр. линии*/
	{
		delay(100);						/*Защита от дребезга*/
		if ((GPIOC->IDR & GPIO_IDR_IDR13) == 0)			// Проверка, что кнопка всё ещё нажата
		{
			TIM2->CR1 ^= TIM_CR1_CEN;					/*XOR (инвертирование) бита включения таймера*/
		}
		EXTI->PR |= EXTI_PR_PR13;						/*Очищаем флаг прерывания, чтобы не зайти в него повторно*/
	}
}
/**
  * @brief  Обработчик прерывания по таймеру 2
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
	LED_NUM_SWAP(led_num);			//Переключение состояния светодиодов (макрос: если было 0, то включаем led_num, иначе выключаем)
	TIM2->SR &= ~TIM_SR_UIF;		/*(status register) UIF - Update Interrupt Flag - очищение флага обновления счета */
}
/**
  * @brief  Инициализация таймера 2 для генерации прерываний с частотой 1кГц
  * @param  None
  * @retval None
  */
void initTIM2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		/*Включаем тактирование на таймере2 шине APB1*/
											/*Нужен 1кГц: делим 64МГц на 64000, потом ставим модуль счета в 1000*/
	TIM2->PSC = 64000 - 1;					/*Prescaler (предделитель): 64'000'000/64'000 = 1'000 (+1 добавляет сам, чтобы не делить на 0)*/
	TIM2->ARR = 1000;						/*Auto-Reload Register, задали 1кГц, т.е. 1с*/
	/*DIER - DMA Interrupt enable register, в нем бит UIE - Update Interrupt Enable - прерывание будет каждый раз при обновлении таймера*/
	TIM2->DIER |= TIM_DIER_UIE;				 //Разрешаем прерывание по обновлению счета
	TIM2->CR1 |= TIM_CR1_CEN;				/*Control register CEN - cnt Enable, разрешаем подсчет*/
	NVIC_EnableIRQ(TIM2_IRQn);				/*Разрешаем NVIC обработку прерываний*/
	NVIC_SetPriority(TIM2_IRQn, 2);			/*Выставляем приоритет 2*/
}
/**
  * @brief  Обновление индикации на дисплее, если значение изменилось
  * @param  newValue - новое значение для отображения (0-9)
  * @retval None
  */
void updateDisplayIfChanged(uint8_t newValue)
{
    static uint8_t lastValue = 255;		 // Хранит предыдущее значение (255 — "ничего ещё не было")

    if (newValue != lastValue)    	 	 // Проверяем, изменилось ли значение
    {
        setDisplay(newValue);       // Обновляем индикацию
		PrintNumDisplay();              // Печатаем текущее значение на UART
        lastValue = newValue;       	// Запоминаем новое значение
    }
}
