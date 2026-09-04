/*
 * eeprom.h
 *
 *  Created on: May 27, 2026
 *      Author: Acadêmico
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_
#include "main.h"

void EEPROM_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef EEPROM_WriteByte( uint16_t mem_addr,uint8_t data);
HAL_StatusTypeDef EEPROM_ReadByte( uint16_t mem_addr,uint8_t *data);
HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t mem_addr, uint8_t *buffer,uint16_t size);
HAL_StatusTypeDef EEPROM_ReadBuffer( uint16_t mem_addr,uint8_t *buffer,uint16_t size);
HAL_StatusTypeDef EEPROM_IsReady(void);

#endif /* INC_EEPROM_H_ */
