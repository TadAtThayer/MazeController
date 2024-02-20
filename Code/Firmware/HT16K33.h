/*
 * HT16K33.h
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#ifndef HT16K33_H_
#define HT16K33_H_

#include <stdint.h>
#include "stm32c0xx_hal.h"

class HT16K33 {
private :

	static uint16_t numberTable[16];

	I2C_HandleTypeDef *_i2c_bus;
	uint8_t _shiftedAddress;
	uint16_t displayBuf[5] = {0};

	bool showBuf( void );

public:

	HT16K33(I2C_HandleTypeDef *bus, uint8_t shiftedAddress = 0x70<<1 );
	virtual ~HT16K33();

	bool turnOn( uint8_t brightness = 8);
	bool turnOff( void );

	bool pass( void );
	bool fail( void );

	bool displayNum( uint8_t nums[4] );
	bool displayHex( uint16_t num );

};

#endif /* HT16K33_H_ */
