#include "eeprom.h"

#define EEPROM_ADDR        (0x50<<1)
#define EEPROM_MAX_ADDR    0x7FFF
#define EEPROM_PAGE_SIZE   64

static I2C_HandleTypeDef *eeprom_i2c;

/*-----------------------------------*/
/* Inicialização                     */
/*-----------------------------------*/

void EEPROM_Init(I2C_HandleTypeDef *hi2c) {
	if (hi2c != NULL) {
		eeprom_i2c = hi2c;
	}
}

/*-----------------------------------*/
/* Verifica disponibilidade EEPROM   */
/*-----------------------------------*/

HAL_StatusTypeDef EEPROM_IsReady(void) {
	if (eeprom_i2c == NULL)
		return HAL_ERROR;
	return HAL_I2C_IsDeviceReady(eeprom_i2c,EEPROM_ADDR,10,1000);
}

/*-----------------------------------*/
/* Escrita 1 byte                    */
/*-----------------------------------*/

HAL_StatusTypeDef EEPROM_WriteByte(uint16_t mem_addr,uint8_t data) {
	if (eeprom_i2c == NULL || mem_addr > EEPROM_MAX_ADDR) {
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Write(eeprom_i2c,EEPROM_ADDR,mem_addr,I2C_MEMADD_SIZE_16BIT,&data,1,1000);
	while (EEPROM_IsReady() != HAL_OK);
	return status;

}

/*-----------------------------------*/
/* Leitura 1 byte                    */
/*-----------------------------------*/

HAL_StatusTypeDef EEPROM_ReadByte(uint16_t mem_addr,uint8_t *data) {
	if (eeprom_i2c == NULL || data == NULL || mem_addr > EEPROM_MAX_ADDR) {
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Read(eeprom_i2c,EEPROM_ADDR,mem_addr,I2C_MEMADD_SIZE_16BIT,data,1,1000);
}

/*-----------------------------------*/
/* Escrita buffer (com paginação)    */
/*-----------------------------------*/

HAL_StatusTypeDef EEPROM_WriteBuffer(uint16_t mem_addr,uint8_t *buffer,uint16_t size) {
	if (eeprom_i2c == NULL || buffer == NULL || (mem_addr + size - 1) > EEPROM_MAX_ADDR) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	while (size > 0) {
		uint16_t page_offset = mem_addr %
		EEPROM_PAGE_SIZE;
		uint16_t bytes_to_write =
		EEPROM_PAGE_SIZE - page_offset;
		if (bytes_to_write > size) {
			bytes_to_write = size;
		}
		status = HAL_I2C_Mem_Write(eeprom_i2c,EEPROM_ADDR,mem_addr,I2C_MEMADD_SIZE_16BIT,buffer,bytes_to_write,1000);
		if (status != HAL_OK) {
			return status;
		}
		while (EEPROM_IsReady() != HAL_OK);
		mem_addr += bytes_to_write;
		buffer += bytes_to_write;
		size -= bytes_to_write;
	}
	return HAL_OK;
}

/*-----------------------------------*/
/* Leitura buffer                    */
/*-----------------------------------*/

HAL_StatusTypeDef EEPROM_ReadBuffer(uint16_t mem_addr,uint8_t *buffer,uint16_t size) {
	if (eeprom_i2c == NULL || buffer == NULL || (mem_addr + size - 1) > EEPROM_MAX_ADDR) {
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Read(eeprom_i2c,EEPROM_ADDR,mem_addr,I2C_MEMADD_SIZE_16BIT,buffer,size,1000);
}
