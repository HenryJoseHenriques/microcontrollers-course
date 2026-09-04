#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

#define TICK_EXPIRED(lastTick, interval) \
    ((HAL_GetTick() - (lastTick) >= (interval)) ? ((lastTick) = HAL_GetTick(), 1) : 0)

uint32_t button_S1_tick = 0, button_S2_tick = 0,button_S3_tick = 0, led_timer_tick = 0;

typedef enum {
	STATE_NONE,
	STATE_START,
	STATE_SET,
	STATE_DEFAULT,
	STATE_INCREASE,
	STATE_TRADE_DIGIT,
	STATE_STOP_RETURN
} estadosTimer;

typedef enum {
	BUTTON_NONE, BUTTON_1, BUTTON_2, BUTTON_3
} button;

estadosTimer next_state, estado = STATE_NONE;
button ev = BUTTON_NONE;

estadosTimer obterProxEstado(estadosTimer atual, button ev) {
	if (atual == STATE_NONE && ev == BUTTON_NONE)
		return STATE_NONE;
	if (atual == STATE_NONE && ev == BUTTON_1)
		return STATE_START;
	if (atual == STATE_NONE && ev == BUTTON_2)
		return STATE_SET;
	//if (atual == STATE_NONE && ev == BUTTON_3) return STATE_CROM;

	if (atual == STATE_START && ev == BUTTON_1)
		return STATE_STOP_RETURN; //return to STATE_START in fuction
	if (atual == STATE_START && ev == BUTTON_2)
		return STATE_SET;
	if (atual == STATE_START && ev == BUTTON_3)
		return STATE_DEFAULT;

	if (atual == STATE_SET && ev == BUTTON_1)
		return STATE_NONE;
	if (atual == STATE_SET && ev == BUTTON_2)
		return STATE_INCREASE; //return to STATE_SET in fuction
	if (atual == STATE_SET && ev == BUTTON_3)
		return STATE_TRADE_DIGIT; //return to STATE_SET in fuction

	if (atual == STATE_STOP_RETURN && ev == BUTTON_1)
		return STATE_START;
	if (atual == STATE_STOP_RETURN && ev == BUTTON_2)
		return STATE_DEFAULT;

    if (atual == STATE_INCREASE)
        return STATE_SET;

    if (atual == STATE_TRADE_DIGIT)
        return STATE_SET;

    if (atual == STATE_DEFAULT)
      return STATE_NONE;

	return atual;
}

button read_button() {
    // Variáveis estáticas para lembrar como o botão estava na última vez que a função rodou
    static GPIO_PinState last_S1 = GPIO_PIN_SET;
    static GPIO_PinState last_S2 = GPIO_PIN_SET;
    static GPIO_PinState last_S3 = GPIO_PIN_SET;

    button pressed = BUTTON_NONE;

    // --- Lógica para o Botão 1 ---
    GPIO_PinState current_S1 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    // Só entra se: AGORA está pressionado (RESET) E ANTES estava solto (SET)
    if (current_S1 == GPIO_PIN_RESET && last_S1 == GPIO_PIN_SET) {
        if (TICK_EXPIRED(button_S1_tick, 50)) {
            pressed = BUTTON_1;
        }
    }
    last_S1 = current_S1; // Salva o estado atual para a próxima comparação

    // --- Lógica para o Botão 2 ---
    GPIO_PinState current_S2 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14);
    if (current_S2 == GPIO_PIN_RESET && last_S2 == GPIO_PIN_SET) {
        if (TICK_EXPIRED(button_S2_tick, 50)) {
            pressed = BUTTON_2;
        }
    }
    last_S2 = current_S2;

    // --- Lógica para o Botão 3 ---
    GPIO_PinState current_S3 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
    if (current_S3 == GPIO_PIN_RESET && last_S3 == GPIO_PIN_SET) {
        if (TICK_EXPIRED(button_S3_tick, 50)) {
            pressed = BUTTON_3;
        }
    }
    last_S3 = current_S3;

    return pressed;
}

void LED_NONE() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void LED_START() {
	 LED_NONE();
     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
}

void LED_SET() {
	LED_NONE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}

void LED_DEFAULT() {
	LED_NONE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_Delay(500);
	//estado = STATE_NONE;
}

void LED_INCREASE() {
	LED_NONE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_Delay(500);
	//estado = STATE_SET;
}

void LED_TRADE_DIGIT() {
	LED_NONE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(500);
	//estado = STATE_SET;
}

void LED_STOP_RETURN() {
	LED_NONE();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void (*exec_logica[])()= {LED_NONE,LED_START,LED_SET,LED_DEFAULT,LED_INCREASE,LED_TRADE_DIGIT,LED_STOP_RETURN};

	int main(void) {
		HAL_Init();
		SystemClock_Config();
		MX_GPIO_Init();

		while (1) {
			//Parte 1: verifica a entrada
			ev = read_button();

			next_state = obterProxEstado(estado, ev);

			estado = next_state;

			//Parte 1: executa a logica repetitiva
			exec_logica[estado]();

		}
	}

	/**
	 * @brief System Clock Configuration
	 * @retval None
	 */
	void SystemClock_Config(void) {
		RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
		RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

		/** Initializes the RCC Oscillators according to the specified parameters
		 * in the RCC_OscInitTypeDef structure.
		 */
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState = RCC_HSE_ON;
		RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
		RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
			Error_Handler();
		}

		/** Initializes the CPU, AHB and APB buses clocks
		 */
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
				| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0)
				!= HAL_OK) {
			Error_Handler();
		}
	}

	/**
	 * @brief GPIO Initialization Function
	 * @param None
	 * @retval None
	 */
	static void MX_GPIO_Init(void) {
		GPIO_InitTypeDef GPIO_InitStruct = { 0 };
		/* USER CODE BEGIN MX_GPIO_Init_1 */
		/* USER CODE END MX_GPIO_Init_1 */

		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(GPIOA,
				GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
						| GPIO_PIN_5, GPIO_PIN_RESET);

		/*Configure GPIO pins : PC13 PC14 PC15 */
		GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/*Configure GPIO pins : PA0 PA1 PA2 PA3
		 PA4 PA5 */
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
				| GPIO_PIN_4 | GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN MX_GPIO_Init_2 */
		/* USER CODE END MX_GPIO_Init_2 */
	}

	/* USER CODE BEGIN 4 */

	/* USER CODE END 4 */

	/**
	 * @brief  This function is executed in case of error occurrence.
	 * @retval None
	 */
	void Error_Handler(void) {
		/* USER CODE BEGIN Error_Handler_Debug */
		/* User can add his own implementation to report the HAL error return state */
		__disable_irq();
		while (1) {
		}
		/* USER CODE END Error_Handler_Debug */
	}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
