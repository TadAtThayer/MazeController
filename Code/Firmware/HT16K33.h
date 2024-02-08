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
	I2C_HandleTypeDef *_i2c_bus;
	uint8_t _shiftedAddress;
	uint16_t displayBuf[5] = {0};
public:


	HT16K33(I2C_HandleTypeDef *bus, uint8_t shiftedAddress = 0x70 );
	virtual ~HT16K33();

	bool turnOn( uint8_t brightness = 8);
	bool turnOff( void );

	bool pass( void );
	bool fail( void );

};

#endif /* HT16K33_H_ */
