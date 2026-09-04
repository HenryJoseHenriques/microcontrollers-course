#include "display.h"

#define DIGITS 4
#define MINUS_SEGMENT 0x40
#define BLANK 254
#define MINUS 255

static uint8_t number[DIGITS];
static uint8_t current_digit = 0;
static uint32_t last_display_tick = 0;

static const uint8_t segments[10] = { 0x3F, //0
		0x06, //1
		0x5B, //2
		0x4F, //3
		0x66, //4
		0x6D, //5
		0x7D, //6
		0x07, //7
		0x7F, //8
		0x6F  //9
		};

#define TICK_EXPIRED(last,interval) \
((HAL_GetTick()-last>=interval)?((last=HAL_GetTick()),1):0)

static void enable_digit(uint8_t digit) {

	GPIOB->BSRR = (GPIO_PIN_12 |
	GPIO_PIN_13 |
	GPIO_PIN_14 |
	GPIO_PIN_15) << 16;

	switch (digit) {

	case 0:
		GPIOB->BSRR =
		GPIO_PIN_12;
		break;

	case 1:
		GPIOB->BSRR =
		GPIO_PIN_13;
		break;

	case 2:
		GPIOB->BSRR =
		GPIO_PIN_14;
		break;

	case 3:
		GPIOB->BSRR =
		GPIO_PIN_15;
		break;

	}

}

void Display_SetTemperature(float temp) {

	int value = (int) temp;

	uint8_t negative = (value < 0);

	if (negative)
		value = -value;

	/*
	 Exemplos:

	 25 -> [blank][blank][2][5]

	 -5 -> [minus][blank][blank][5]

	 125 -> [blank][1][2][5]

	 */

	number[0] = value % 10;

	number[1] = (value / 10) % 10;

	number[2] = (value / 100) % 10;

	number[3] =
	BLANK;

	if (value < 10) {

		number[1] = BLANK;
		number[2] = BLANK;

	} else if (value < 100) {

		number[2] = BLANK;

	}

	if (negative) {

		number[3] = MINUS;

	}

}

void Display_Refresh(void) {

	if (TICK_EXPIRED(last_display_tick, 2)) {

		GPIOA->BSRR = 0x00FF << 16;

		if (number[current_digit] <= 9) {

			GPIOA->BSRR = segments[number[current_digit]];

		} else if (number[current_digit] ==
		MINUS) {

			GPIOA->BSRR =
			MINUS_SEGMENT;

		}
		enable_digit(current_digit);
		current_digit = (current_digit + 1) % DIGITS;

	}

}
