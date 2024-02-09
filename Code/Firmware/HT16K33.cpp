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
	uint8_t command = 0;

	//                .gfedcbazzzzzzzz
	displayBuf[0] = 0b0111001100000000;
	displayBuf[1] = 0b0111011100000000;
	displayBuf[2] = 0;
	displayBuf[3] = 0b0110110100000000;
	displayBuf[4] = 0b0110110100000000;

	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100);
	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, (uint8_t*)&displayBuf, 10, 1000);

	return true;
}

bool HT16K33::fail( void ){
	// Write fail to the display
	// Assume on already
	uint8_t command = 0;

	//                .gfedcbazzzzzzzz
	displayBuf[0] = 0b0111000100000000;
	displayBuf[1] = 0b0111011100000000;
	displayBuf[2] = 0;
	displayBuf[3] = 0b0011000000000000;
	displayBuf[4] = 0b0011100000000000;

	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100);
	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, (uint8_t*)&displayBuf, 10, 1000);

	return true;
}


