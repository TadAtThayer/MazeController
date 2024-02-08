/*
 * HT16K33.cpp
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#include "HT16K33.h"

HT16K33::HT16K33( I2C_HandleTypeDef *bus, uint8_t shiftedAddress ) {
	// TODO Auto-generated constructor stub
	_shiftedAddress = shiftedAddress;
	_i2c_bus = bus;
}

HT16K33::~HT16K33() {
	// TODO Auto-generated destructor stub
	turnOff();
}

bool HT16K33::turnOn(uint8_t brightness){
	uint8_t command = 0x21;   // Turn on the oscillator

	if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100) != HAL_OK) return false;

	// Now set the brightness
	command = 0xE0 | (brightness & 0x0F);  // lots of magic here...need some defines
	if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100) != HAL_OK) return false;

	command = 0x81;  // Display on
	if ( HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100) != HAL_OK) return false;

	return true;
}

bool HT16K33::turnOff(){
	uint8_t command = 0x80;   // Turn off the display

	if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100) != HAL_OK) return false;

	command = 0x20;  // Oscillator off
	if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100) != HAL_OK) return false;

	return true;
}

bool HT16K33::pass( void ){
	// Write Pass to the display
	// Assume on already

	//                .gfedcba
	displayBuf[0] = 0x7300;
	displayBuf[1] = 0x7700;
	displayBuf[2] = 0x0000;
	displayBuf[3] = 0x6C00;
	displayBuf[4] = 0x6C00;


	for ( int i = 0; i < 10; i++ ){
		uint8_t addr = i;
		if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &addr, 1, 100) != HAL_OK) return false;
		if (HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, (uint8_t*)&displayBuf[i], 2, 100) != HAL_OK) return false;
	}
	return true;
}
