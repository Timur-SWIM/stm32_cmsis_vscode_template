#include "uart.h"
#include <stdbool.h>

char RxBuffer[RX_BUFF_SIZE];			//Буфер приёма USART
char TxBuffer[TX_BUFF_SIZE];			//Буфер передачи USART
volatile bool ComReceived;				//Флаг приёма строки данных
volatile uint8_t RxIndex = 0; 			//Индекс для приёма данных

/**
  * @brief  Обработчик прерывания по USART2
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	if ((USART2->SR & USART_SR_RXNE)!=0)		//Прерывание по приёму данных
	{
		uint8_t pos = strlen(RxBuffer);			//Вычисляем позицию свободной ячейки

		RxBuffer[pos] = USART2->DR;				//Считываем содержимое регистра данных

		if ((RxBuffer[pos]== 0x0A) && (RxBuffer[pos-1]== 0x0D))							//Если это символ конца строки
		{
			ComReceived = true;					//- выставляем флаг приёма строки
			return;								//- и выходим
		}
	}
}
/**
  * @brief  Инициализация USART2
  * @param  None
  * @retval None
  */
void initUSART2(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;						//включить тактирование альтернативных ф-ций портов
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					//включить тактирование UART2

	GPIOA->CRL &= ~(GPIO_CRL_MODE2 | GPIO_CRL_CNF2);		//PA2 на выход
	GPIOA->CRL |= (GPIO_CRL_MODE2_1 | GPIO_CRL_CNF2_1);

	GPIOA->CRL &= ~(GPIO_CRL_MODE3 | GPIO_CRL_CNF3);		//PA3 - вход
	GPIOA->CRL |= GPIO_CRL_CNF3_0;

	/*****************************************
	Скорость передачи данных - 57600 бод
	Частота шины APB1 - 32МГц

	1. USARTDIV = 32'000'000/(16*57600) = 34,7222
	2. 34 = 0x22
	3. 16*0.7 = 11,2 ~ 11 = 0xB
	4. Итого 0x22B
	*****************************************/
	USART2->BRR = 0x22B;

	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
	USART2->CR1 |= USART_CR1_RXNEIE;						//разрешить прерывание по приему байта данных

	NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief  Передача строки по USART2 без DMA
  * @param  *str - указатель на строку
  * @param  crlf - если true, перед отправкой добавить строке символы конца строки
  * @retval None
  */
void txStr(char *str, bool crlf)
{
	uint16_t i;

	if (crlf)												//если просят,
		strcat(str,"\r\n");									//добавляем символ конца строки

	for (i = 0; i < strlen(str); i++)
	{
		USART2->DR = str[i];								//передаём байт данных
		while ((USART2->SR & USART_SR_TC)==0) {};			//ждём окончания передачи
	}
}
/**
  * @brief  Обработчик команд
  * @param  None
  * @retval None
  */
void ExecuteCommand(void)
{
	memset(TxBuffer,0,sizeof(TxBuffer));					//Очистка буфера передачи

	/* Обработчик команд */
	if (strncmp(RxBuffer,"*IDN?",5) == 0)					//Это команда "*IDN?"
	{
		strcpy(TxBuffer,"Kupriyanov M. M., Myaldzin T. R., IU4-73B");
	}
	else if (strncmp(RxBuffer,"SET",3) == 0)				//Команда запуска таймера?
	{
		uint16_t set_value;
		sscanf(RxBuffer,"%*s %hu", &set_value);
		if ((set_value <= 9))		//параметр должен быть в заданных пределах!
		{
			TIM3->CNT = set_value * 2 + 18; //!!!!
			strcpy(TxBuffer, "OK");
		}
		else
			strcpy(TxBuffer, "Parameter is out of range");
	}
	else if (strncmp(RxBuffer,"GET",3) == 0)				//Команда остановки таймера?
	{
		uint32_t counter_value = (TIM3->CNT - 18) / 2; //!!!!
		sniprintf(TxBuffer, sizeof(TxBuffer), "%lu", counter_value); //!!!!
	}
	else if (strncmp(RxBuffer,"PERIOD",6) == 0)				//Команда изменения периода таймера?
	{
		uint16_t tim_value;
		sscanf(RxBuffer,"%*s %hu", &tim_value);				//преобразуем строку в целое число

		if ((100 <= tim_value) && (tim_value <= 5000))		//параметр должен быть в заданных пределах!
		{
			TIM2->ARR = tim_value;  //!!!!
			TIM2->CNT = 0;          //!!!!

			strcpy(TxBuffer, "OK");
		}
		else
			strcpy(TxBuffer, "Parameter is out of range");	//ругаемся
	}
	else
		strcpy(TxBuffer,"Invalid Command");					//Если мы не знаем, чего от нас хотят, ругаемся в ответ

	txStr(TxBuffer, true);
	memset(RxBuffer,0,RX_BUFF_SIZE);						//Очистка буфера приёма
	ComReceived = false;									//Сбрасываем флаг приёма строки
}

/**
  * @brief  Печать текущего значения счётчика энкодера
  * @param  None
  * @retval None
  */
void PrintNumDisplay(void) {
	uint32_t counter_value = TIM3->CNT - 18;
	counter_value >>= 1;
	sniprintf(TxBuffer, sizeof(TxBuffer), "%ld", counter_value);
	txStr(TxBuffer, true);
}

/**
  * @brief  Проверка флага приёма данных
  * @param  None
  * @retval true - данные получены, false - данных нет
  */
bool COM_RECEIVED(void)
{
    return ComReceived;
}
