/*
 * lm75.c
 *
 *  Created on: May 23, 2026
 *      Author: Acadêmico
 */
#include "lm75.h"

/*
Funções síncronas (bloqueantes) — sempre disponíveis:
Função Descrição
HAL_I2C_Master_Transmit			Envia dados diretamente a um escravo
HAL_I2C_Master_Receive			Recebe dados diretamente de um escravo
HAL_I2C_Mem_Write				Escreve em registrador interno
HAL_I2C_Mem_Read				Lê de registrador interno
HAL_I2C_IsDeviceReady			Verifica se o escravo está respondendo
HAL_I2C_GetState				Retorna o estado atual do I2C
HAL_I2C_GetError				Retorna o código de erro da última operação
*/
I2C_HandleTypeDef hi2c1;

void LM75_Init(I2C_HandleTypeDef *hi2c1){

	  /* USER CODE BEGIN I2C1_Init 0 */

	  /* USER CODE END I2C1_Init 0 */

	  /* USER CODE BEGIN I2C1_Init 1 */

	  /* USER CODE END I2C1_Init 1 */
	  hi2c1->Instance = I2C1;
	  hi2c1->Init.ClockSpeed = 400000;
	  hi2c1->Init.DutyCycle = I2C_DUTYCYCLE_2;
	  hi2c1->Init.OwnAddress1 = 0;
	  hi2c1->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	  hi2c1->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	  hi2c1->Init.OwnAddress2 = 0;
	  hi2c1->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	  hi2c1->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	  if (HAL_I2C_Init(&*hi2c1) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  /* USER CODE BEGIN I2C1_Init 2 */

	  /* USER CODE END I2C1_Init 2 */
}

float LM75_ConvertTemperature(uint16_t temp){
	return 0.125*((float)(temp>>5));
}

HAL_StatusTypeDef LM75_ReadTemperature(float *temperature){
	uint8_t buffer[2];
	uint8_t reg_ptr = 0x00;
	uint16_t tempRead;
	//Escreve o endereço e o pointer byter do escravo
	if(HAL_I2C_Master_Transmit(&hi2c1, 0x90, &reg_ptr, 1, 100) != HAL_OK){
		return HAL_TIMEOUT;
	}
	//Recebe o valor da temperatura e armazena na variável buffer
	if(HAL_I2C_Master_Receive(&hi2c1, 0x90, buffer, 2, 100)!= HAL_OK){
		return HAL_TIMEOUT;
	}
	tempRead = (((uint16_t)buffer[0])<<8) + buffer[1];
	*temperature = LM75_ConvertTemperature(tempRead);
	return HAL_OK;
}

HAL_StatusTypeDef LM75_SetShutdown(uint8_t enable){

}


