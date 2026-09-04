#include "lm75.h"

#define LM75_ADDR      (0x48 << 1)
#define LM75_TEMP_REG  0x00
#define LM75_CONF_REG  0x01
#define LM75_TIMEOUT   1000

static I2C_HandleTypeDef *lm75_i2c;

static float LM75_ConvertTemperature(uint16_t temp) {
	int16_t signed_temp = (int16_t) temp;

	signed_temp >>= 5;

	return signed_temp * 0.125f;
}

void LM75_Init(I2C_HandleTypeDef *hi2c) {
	if (hi2c != NULL) {
		lm75_i2c = hi2c;
	}
}

HAL_StatusTypeDef LM75_ReadTemperature(float *temperature) {

	if (lm75_i2c == NULL || temperature == NULL) {
		return HAL_ERROR;
	}

	uint8_t buffer[2];
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read(lm75_i2c,LM75_ADDR,LM75_TEMP_REG,I2C_MEMADD_SIZE_8BIT,buffer,2,LM75_TIMEOUT);
	if (status != HAL_OK)
		return status;

	uint16_t temp_raw =((uint16_t) buffer[0] << 8)|buffer[1];
	*temperature = LM75_ConvertTemperature(temp_raw);

	return HAL_OK;
}

HAL_StatusTypeDef LM75_SetShutdown(uint8_t enable) {
	if (lm75_i2c == NULL) {
		return HAL_ERROR;
	}

	uint8_t conf;

	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(lm75_i2c,LM75_ADDR,LM75_CONF_REG,I2C_MEMADD_SIZE_8BIT,&conf,1,LM75_TIMEOUT);

	if (status != HAL_OK)
		return status;

	if (enable)
		conf |= 0x01;

	else
		conf &= ~0x01;

	return HAL_I2C_Mem_Write(lm75_i2c,LM75_ADDR,LM75_CONF_REG,I2C_MEMADD_SIZE_8BIT,&conf,1,LM75_TIMEOUT);
}
