#include "uart.h"

char RxBuffer[RX_BUFF_SIZE];			//Буфер приёма USART
char TxBuffer[TX_BUFF_SIZE];			//Буфер передачи USART
volatile bool ComReceived;				//Флаг приёма строки данных

volatile uint8_t RxIndex = 0;

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
	Скорость передачи данных - 19200
	Частота шины APB1 - 32МГц

	1. USARTDIV = 32'000'000/(16*19200) = 104,1667
	2. 104 = 0x68
	3. 16*0.2 = 3
	4. Итого 0x683
	*****************************************/
	USART2->BRR = 0x683;

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
//	txStr(RxBuffer, false);
	memset(TxBuffer,0,sizeof(TxBuffer));					//Очистка буфера передачи

	/* Обработчик команд */
	if (strncmp(RxBuffer,"*IDN?",5) == 0)					//Это команда "*IDN?"
	{
		//Она самая, возвращаем строку идентификации
		strcpy(TxBuffer,"It'ssa me, Mario!");

	}
	else if (strncmp(RxBuffer,"SET",3) == 0)				//Команда запуска таймера?
	{
		uint16_t set_value;
		sscanf(RxBuffer,"%*s %hu", &set_value);
		if ((0 <= set_value) && (set_value <= 9))		//параметр должен быть в заданных пределах!
		{
			TIM3->CNT = set_value; //!!!!
			strcpy(TxBuffer, "OK");
		}
		else
			strcpy(TxBuffer, "Parameter is out of range");

		strcpy(TxBuffer, "OK");
	}
	else if (strncmp(RxBuffer,"GET",3) == 0)				//Команда остановки таймера?
	{
		uint32_t counter_value = TIM3->CNT;
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

	// Передача принятой строки обратно одним из двух способов
	#ifdef USE_DMA
		txStrWithDMA(TxBuffer, true);
	#else
		txStr(TxBuffer, true);
	#endif

	memset(RxBuffer,0,RX_BUFF_SIZE);						//Очистка буфера приёма
	ComReceived = false;									//Сбрасываем флаг приёма строки
}

bool COM_RECEIVED(void)
{
    return ComReceived;
}
