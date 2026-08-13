/*
 * lm75.h
 *
 *  Created on: May 23, 2026
 *      Author: Acadêmico
 */

#ifndef INC_LM75_H_
#define INC_LM75_H_
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"

//float LM75_ADC_Temperature_Calculete(uint16_t adcResult);
void LM75_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LM75_ReadTemperature(float *temperature);
HAL_StatusTypeDef LM75_SetShutdown(uint8_t enable);

#endif /* INC_LM75_H_ */
