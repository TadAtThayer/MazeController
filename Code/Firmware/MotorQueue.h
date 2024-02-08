/*
 * MotorQueue.h
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#ifndef MOTORQUEUE_H_
#define MOTORQUEUE_H_

#include <stdint.h>
#include "Registers.h"

// This is gross...  Should move to a BSP
#include "stm32c0xx_hal.h"

#include "MoveQueue.h"

class MotorQueue: public MoveQueue {
private:
	Registers *registers = &SystemRegisters;
	const uint16_t *coilActivations;
	bool fullStep = true;
	uint16_t pinMask;
	GPIO_TypeDef *gpioPort;
	int stepsTaken = 0;
	int quadrant = 0;
	uint32_t lastMoveTime = 0;

public:
	MotorQueue(const uint16_t *coils, GPIO_TypeDef *port, uint16_t pins,
			bool fs = true);
	void idle();
	bool halfStep();
	void step(void);
};

#endif /* MOTORQUEUE_H_ */
