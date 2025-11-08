#include "display.h"
#include "main.h"
char led_num = 0x003F;

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

	SET_LED_NUM(led_num);
}

void initLED(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;	/*	Включаем тактирование порта C	*/
	GPIOC->CRL &=~GPIO_CRL_CNF0;		/*	CNF=00 push-pull для 0 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE0;		/*	MODE=11, Output 50 MHZ для 0 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF1;		/*	CNF=00 push-pull для 1 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE1;		/*	MODE=11, Output 50 MHZ для 1 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF2;		/*	CNF=00 push-pull для 2 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE2;		/*	MODE=11, Output 50 MHZ для 2 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF3;		/*	CNF=00 push-pull для 3 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE3;		/*	MODE=11, Output 50 MHZ для 3 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF4;		/*	CNF=00 push-pull для 4 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE4;		/*	MODE=11, Output 50 MHZ для 4 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF5;		/*	CNF=00 push-pull для 5 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE5;		/*	MODE=11, Output 50 MHZ для 5 пина	*/
	GPIOC->CRL &=~GPIO_CRL_CNF6;		/*	CNF=00 push-pull для 6 пина	*/
	GPIOC->CRL |= GPIO_CRL_MODE6;		/*	MODE=11, Output 50 MHZ для 6 пина	*/
}

void init_button(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;	/*	Включаем тактирование порта C	*/
	AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI13_PC;					/*	Кнопка на PC13, поэтому настраиваем AFIO_EXTI4, соотв. этому порту	*/
	EXTI->IMR |= EXTI_IMR_MR13;									/*	Изначально все прерывания запрещены, поэтому надо в маску IMR в 13й бит записать 1	*/
	EXTI->FTSR |= EXTI_FTSR_TR13;								/*RTSR - прерывание по нарастанию, FTSR - прерывание по спаду*/
	/*Теперь каждый раз при нажатии кнопки будет генерироваться прерывание по спаду, и этот сигнал будет подаваться в контроллер прерываний - NVIC*/
	/*Но мы не разрешили NVIC их обрабатывать, поэтому он ничего не будет делать*/
	NVIC_EnableIRQ(EXTI15_10_IRQn);								/*Разрешили обработку прерываний с линий с 10 по 15*/
	NVIC_SetPriority(EXTI15_10_IRQn, 1);						/*Выставили единственному прерыванию самый большой приоритет*/
}

void EXTI15_10_IRQHandler(void)									/*Функция для обработки прерываний, вызывается автоматически*/
{
	if ((EXTI->PR & EXTI_PR_PR13) != 0)							/*PR - регистр, каждый бит которого соотв. флагу прерывания на опр. линии*/
	{
		delay(100);												/*Защита от дребезга*/
		if ((GPIOC->IDR & GPIO_IDR_IDR13) == 0)
		{
			TIM2->CR1 ^= TIM_CR1_CEN;							/*XOR (инвертирование) бита включения таймера*/
		}
		EXTI->PR |= EXTI_PR_PR13;								/*Очищаем флаг прерывания, чтобы не зайти в него повторно*/
	}
}

void TIM2_IRQHandler(void)
{
	LED_NUM_SWAP(led_num);
	TIM2->SR &= ~TIM_SR_UIF;									/*UIF - Update Interrupt Flag - программное очищение флага путем записывания нуля*/
}

void initTIM2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		/*Включаем тактирование на шине APB1, на которой таймер*/
											/*Нужен 1Гц: делим 64МГц на 64000, потом на 1000*/
	TIM2->PSC = 64000 - 1;					/*Настройка Prescaler - предделитель*/
	TIM2->ARR = 1000;						/*ARR - Auto-Reload Register, задали 1Гц, т.е. 1с*/
	/*DIER - DMA Interrupt enable register, в нем бит UIE - Update Interrupt Enable - прерывание будет каждый раз при обновлении таймера*/
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CR1 |= TIM_CR1_CEN;				/*CEN - cnt Enable, разрешаем подсчет*/
	NVIC_EnableIRQ(TIM2_IRQn);				/*Разрешаем NVIC обработку прерываний*/
	NVIC_SetPriority(TIM2_IRQn, 2);			/*Выставляем приоритет единицу*/
}

void updateDisplayIfChanged(uint8_t newValue)
{
    static uint8_t lastValue = 255; // Хранит предыдущее значение (255 — "ничего ещё не было")

    if (newValue != lastValue)      // Проверяем, изменилось ли значение
    {
        setDisplay(newValue);       // Обновляем индикацию
        lastValue = newValue;       // Запоминаем новое значение
    }
}
