/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include <stdint.h>
#define DIGITS 4
#define LIM_MIN 99
#define LIM_SEC 59
#define TICK_EXPIRED(lastTick, interval) \
    ((HAL_GetTick() - (lastTick) >= (interval)) ? ((lastTick) = HAL_GetTick(), 1) : 0)
#define PART_ONE(n) (n % 10)
#define PART_TEN(n) ((n / 10) % 10)

//static uint32_t last_timer_tick = HAL_GetTick();
uint32_t last_timer_tick = 0;
uint32_t last_display_tick = 0;
uint32_t last_S1_tick = 0;
uint32_t last_S2_tick = 0;
uint32_t last_S3_tick = 0;
uint8_t current_digit = 0;
//const uint32_t display_interval = 2;
volatile uint8_t number[DIGITS];

uint8_t min, sec;
const uint8_t segments[10] = {
        0x3F, // 0
		0x06, // 1
		0x5B, // 2
		0x4F, // 3
		0x66, // 4
		0x6D, // 5
		0x7D, // 6
		0x07, // 7
		0x7F, // 8
		0x6F  // 9
};

//Cabeçalhos
 void default_timer(void);
 void set_timer(void);
 void split_number(void);
 void enable_digit(uint8_t digit);
 void refrash_display(void);
 void start_timer(void);

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  default_timer();
  split_number();
  while (1)
  {
    /* USER CODE END WHILE */
	  start_timer();
	  //enable_digit(1);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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


 void default_timer(void){
    min = 5;
    sec = 0;
}

 void set_timer(void){
    uint8_t sec_one = PART_ONE(sec);
    uint8_t sec_ten = PART_TEN(sec);
    uint8_t min_one = PART_ONE(min);
    uint8_t min_ten = PART_TEN(min);
    uint8_t count = 0;

    // Loop principal: sai quando apertar S1 (PIN 13)
    // Assumindo que o botão em repouso é SET (1) e pressionado é RESET (0)
    while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET){

        // --- Lógica do Botão S2 (Troca Dígito) ---
        if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14) == GPIO_PIN_RESET){
            if(TICK_EXPIRED(last_S2_tick, 200)){
                count = (count + 1) % 4; // Cicla entre 0, 1, 2, 3
            }
        }

        // --- Lógica do Botão S3 (Incrementa Valor) ---
        if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET){
            if(TICK_EXPIRED(last_S3_tick, 200)){
                switch(count){
                    case 0: if(++sec_one > 9) sec_one = 0; break;
                    case 1: if(++sec_ten > 5) sec_ten = 0; break;
                    case 2: if(++min_one > 9) min_one = 0; break;
                    case 3: if(++min_ten > 9) min_ten = 0; break;
                }
            }
        }

        // Atualiza os valores temporários para o display
        //uint8_t min_temp = min_ten * 10 + min_one;
        //uint8_t sec_temp = sec_ten * 10 + sec_one;

        // Aqui você chamaria a função que atualiza o buffer do display
        // split_number(min_temp, sec_temp);

        refrash_display();
    }

    // Ao sair do loop (S1 pressionado), salva os valores globais
    min = min_ten * 10 + min_one;
    sec = sec_ten * 10 + sec_one;

    // Pequeno delay para evitar que o clique do S1 dispare outras funções ao sair
    HAL_Delay(200);
}

 void split_number(void){
    // if(min > LIM_MIN) min_split = LIM_MIN;
    // if(sec > LIM_SEC) sec_split = LIM_SEC;
    number[0] = PART_TEN(min);
    number[1] = PART_ONE(min);
    number[2] = PART_TEN(sec);
    number[3] = PART_ONE(sec);
}

 void enable_digit(uint8_t digit){
    GPIOB->ODR = (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) << 16;
    switch(digit){
        case 0: GPIOB->ODR = GPIO_PIN_12; break;
        case 1: GPIOB->ODR = GPIO_PIN_13; break;
        case 2: GPIOB->ODR = GPIO_PIN_14; break;
        case 3: GPIOB->ODR = GPIO_PIN_15; break;
    }
}
 void refrash_display(void) {
    if (TICK_EXPIRED(last_display_tick, 2)) {
        //last_display_tick = HAL_GetTick();

        // 1. Desliga todos os dígitos (Porta B) para evitar "fantasma" entre trocas
        GPIOB->ODR = (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15) << 16;
        GPIOA->ODR = (GPIOA->ODR & 0xFF00) | segments[number[current_digit]];
        enable_digit(current_digit);
        current_digit = (current_digit + 1) % DIGITS;
    }
}

 void start_timer(void){
    if (TICK_EXPIRED(last_timer_tick, 1000))
    {
        //last_timer_tick = HAL_GetTick(); // Reinicia a contagem
         if(sec > 0){
            sec--;
        }else{
            sec = LIM_SEC;
            min--;
            //if(min <= 0){
            //    //LED BRANCO
            //    return;
            //}
            split_number();
        }
         refrash_display();
    }
}
