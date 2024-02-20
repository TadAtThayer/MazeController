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

uint16_t HT16K33::numberTable[] = {
		  0x3F00, /* 0 */
		  0x0600, /* 1 */
		  0x5B00, /* 2 */
		  0x4F00, /* 3 */
		  0x6600, /* 4 */
		  0x6D00, /* 5 */
		  0x7D00, /* 6 */
		  0x0700, /* 7 */
		  0x7F00, /* 8 */
		  0x6F00, /* 9 */
		  0x7700, /* a */
		  0x7C00, /* b */
		  0x3900, /* C */
		  0x5E00, /* d */
		  0x7900, /* E */
		  0x7100, /* F */
};

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

bool HT16K33::showBuf( void ) {
	uint8_t command = 0;
	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, &command, 1, 100);
	HAL_I2C_Master_Transmit(_i2c_bus, _shiftedAddress, (uint8_t*)&displayBuf, 10, 1000);

	return true;

}

bool HT16K33::pass( void ){
	//                .gfedcbazzzzzzzz
	displayBuf[0] = 0b0111001100000000;
	displayBuf[1] = 0b0111011100000000;
	displayBuf[2] = 0;
	displayBuf[3] = 0b0110110100000000;
	displayBuf[4] = 0b0110110100000000;
	showBuf();
	return true;
}

bool HT16K33::fail( void ){
	//                .gfedcbazzzzzzzz
	displayBuf[0] = 0b0111000100000000;
	displayBuf[1] = 0b0111011100000000;
	displayBuf[2] = 0;
	displayBuf[3] = 0b0011000000000000;
	displayBuf[4] = 0b0011100000000000;
	showBuf();
	return true;
}

bool HT16K33::displayNum(uint8_t nums[] ){
	displayBuf[0] = numberTable[nums[0]];
	displayBuf[1] = numberTable[nums[1]];
	displayBuf[2] = 0;
	displayBuf[3] = numberTable[nums[2]];
	displayBuf[4] = numberTable[nums[3]];

}

bool HT16K33::displayHex( uint16_t num ){
	displayBuf[0] = numberTable[(num & 0xf000) >> 12];
	displayBuf[1] = numberTable[(num & 0x0f00) >> 8];
	displayBuf[3] = numberTable[(num & 0x00f0) >> 4];
	displayBuf[4] = numberTable[(num & 0x000f) >> 0];
	displayBuf[2] = 0;
	showBuf();
	return true;
}


