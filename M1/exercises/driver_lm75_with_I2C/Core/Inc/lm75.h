/*
 * lm75.h
 *
 *  Created on: May 23, 2026
 *      Author: Acadêmico
 */

#ifndef INC_LM75_H_
#define INC_LM75_H_
#include "main.h"
#include <stdint.h>


/*FUNÇÕES DO LM75*/
void LM75_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef LM75_ReadTemperature(float *temperature);
HAL_StatusTypeDef LM75_SetShutdown(uint8_t enable);

#endif /* INC_LM75_H_ */
