/*
 * eeprom24aa256.h
 *
 *  Created on: May 23, 2026
 *      Author: Acadêmico
 */

#ifndef INC_EEPROM24AA256_H_
#define INC_EEPROM24AA256_H_
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"
void EEPROM_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef EEPROM_WriteByte(uint16_t mem_addr,uint8_t data);
HAL_StatusTypeDef EEPROM_ReadByte( uint16_t mem_addr,uint8_t *data);
HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t mem_addr,uint8_t *buffer,uint16_t size);
HAL_StatusTypeDef EEPROM_ReadBuffer( uint16_t mem_addr,uint8_t *buffer,uint16_t size);
HAL_StatusTypeDef EEPROM_IsReady(void);


#endif /* INC_EEPROM24AA256_H_ */
