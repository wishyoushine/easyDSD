/*
	Direct Stream Digital (DSD) player - hardware glue. 
	Glue code for your specific hardware (initially designed for STM32F407VE).

	License: GPL 3.0
	Copyright (C) 2020 Lukas Neverauskis
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hwg.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

DebugSignal DEBUG_SIG;
SPI spi;
I2S i2s;

/************************************************/
/*			Arduino-like Print class			*/
/************************************************/
void Print::printDebug(char * const buffer, uint32_t size) {

	for(auto buff = buffer; buff != buffer + size; buff++) {

		switch(*buff) {
			case '\0': print("_");break;
			case '\n': print("/");break;
			default: write(*buff); break;
		}
	}
}

void Print::print(unsigned char c) {
	write(c);
}

void Print::print(String str) {
/* deprecated - for <string> todo */
//	for (auto it = str.begin(); it != str.end(); ++it) {
//		write(*it);
//	}
	for (char const * it = str; *it != '\0'; it++)
		write(*it);

}
void Print::println(String str) {
	print(str);
	write('\n');
}

void Print::print(const float value) {
	char str[30] = { 0 };
	sprintf(str, "%.2f", value);
	print(str);
}

void Print::println(const float value) {
	print(value);
	write('\n');
}

//todo: not the fastest way of digital write/read. Needs to fetch from array...

const tft_pinout_stm32 tft_stm32_pinmap[] = {
	{TFT_CS_GPIO_Port, TFT_CS_Pin},
	{TFT_RESET_GPIO_Port, TFT_RESET_Pin},
	{TFT_SDA_GPIO_Port, TFT_SDA_Pin},
	{TFT_SCK_GPIO_Port, TFT_SCK_Pin},
	{TFT_A0_GPIO_Port, TFT_A0_Pin}
};

/************************************************/
/*						SPI						*/
/************************************************/

bool SPI::_dmaBusy = false;
SPI_HandleTypeDef* SPI::_hspi = &hspi1;


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

	if(hspi == SPI::_hspi) {

		SPI::_dmaBusy = false;
		HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);
	}
}

int8_t SPI::transfer(uint8_t* data, dc_mode_e dc, uint32_t size) {

	int8_t result = -1;
	if(!_dmaBusy) {
		_dmaBusy = true;
		HAL_GPIO_WritePin(TFT_A0_GPIO_Port, TFT_A0_Pin, dc == DC_COMMAND ? GPIO_PIN_RESET:GPIO_PIN_SET);
		HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET);
		result = -1*HAL_SPI_Transmit_DMA(_hspi, data, size);
	}
	return result;
}

/* Glue code template for user to fill */

void digitalWrite(uint8_t pin, boolean state) {

	HAL_GPIO_WritePin(tft_stm32_pinmap[pin].GPIOx, tft_stm32_pinmap[pin].GPIO_Pin, (GPIO_PinState)state);

}

boolean digitalRead(uint8_t pin) {

	return HAL_GPIO_ReadPin(tft_stm32_pinmap[pin].GPIOx, tft_stm32_pinmap[pin].GPIO_Pin);

}

/************************************************/
/*						I2S						*/
/************************************************/

i2s_state_e I2S::_state = {I2S_UNKNOWN};

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	DEBUG_SIG.set(3);
	I2S::_state = I2S_STREAMING_PONG;
/*	static I2S_HandleTypeDef * phI2S_waiter = NULL;

	if(!phI2S_waiter) {

		HAL_I2S_DMAPause(hi2s);
		phI2S_waiter = hi2s;
	}
	else {
		DEBUG_SIG.set(3);
		I2S::_state = I2S_STREAMING_PONG;
		HAL_I2S_DMAResume(phI2S_waiter);
		phI2S_waiter = NULL;
	}*/
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{

	DEBUG_SIG.reset(3);
	I2S::_state = I2S_STREAMING_PING;

/*	static I2S_HandleTypeDef * phI2S_waiter = NULL;

	if(!phI2S_waiter) {

		HAL_I2S_DMAPause(hi2s);
		phI2S_waiter = hi2s;
	}
	else {
		DEBUG_SIG.reset(3);
		I2S::_state = I2S_STREAMING_PING;
		HAL_I2S_DMAResume(phI2S_waiter);
		phI2S_waiter = NULL;
	}*/
}

/************************************************/
/*					I2S END						*/
/************************************************/


