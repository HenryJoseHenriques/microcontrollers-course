/*
 * eeprom24aa256.c
 *
 *  Created on: May 23, 2026
 *      Author: Acadêmico
 */

#include "eeprom24aa256.h"
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


void EEPROM_Init(I2C_HandleTypeDef *hi2c1){

}

HAL_StatusTypeDef EEPROM_WriteByte(uint16_t mem_addr,uint8_t data){

}

HAL_StatusTypeDef EEPROM_ReadByte( uint16_t mem_addr,uint8_t *data){

}

HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t mem_addr,uint8_t *buffer,uint16_t size){

}

HAL_StatusTypeDef EEPROM_ReadBuffer( uint16_t mem_addr,uint8_t *buffer,uint16_t size){

}

HAL_StatusTypeDef EEPROM_IsReady(void){

}
