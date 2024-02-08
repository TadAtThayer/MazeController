/*
 * MotorQueue.cpp
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#include "MotorQueue.h"

// Yuck - should be in a BSP
#include <stm32c0xx_hal_gpio.h>

	MotorQueue::MotorQueue(const uint16_t *coils, GPIO_TypeDef *port, uint16_t pins, bool fs) {
		coilActivations = coils;
		pinMask = pins;
		gpioPort = port;
		fullStep = fs;
	}

	void MotorQueue::idle() {
		// Turn the coils off
		HAL_GPIO_WriteMultipleStatePin(gpioPort, pinMask, 0);
	}

	bool MotorQueue::halfStep() {
		return !fullStep;
	}

	void MotorQueue::step(void) {
		int incVal = 1;

		if (!isEmpty()) {
			lastMoveTime = HAL_GetTick();
			if (stepsTaken > peek()) {
				// We're going backward.
				incVal = -1;
			}

			// Drive the motors
			HAL_GPIO_WriteMultipleStatePin(gpioPort, pinMask,
					coilActivations[quadrant & (fullStep ? 0x3 : 0x7)]);
			quadrant += incVal;
			stepsTaken += incVal;

			if (stepsTaken == peek()) {
				// We're done with the move at the head of the queue
				//  pop it and start the next.
				pop();
				stepsTaken = 0;
			}

		} else {
			if (HAL_GetTick() - lastMoveTime > registers->offTime) {
				lastMoveTime = HAL_GetTick();
				idle();
			}
		}
	}
