/*
 * MazeMain.cpp
 *
 *  Created on: Jan 9, 2024
 *      Author: d66317d
 */
#include "main.h"
#include <stdint.h>
#include <assert.h>

#include "Registers.h"
#include "MoveQueue.h"
#include "MotorQueue.h"
#include "HT16K33.h"

// These motors (28BYJ-48) are supposedly 32 steps per revolution followed by
//  a 64:1 gearbox.  So, effectively 2048 full steps.  My experiments indicate
//  it is actually 512
#define PPR 512

// The pulse detector will report the pulse length
// in microseconds ranging from 1000 to 2000.  This is
// 180 degrees of arc.
//
// There are approximately 5.6 activations per degree.
//
// 2032 steps / 360 degrees = 5.64 phases per degree.
//
// Now we need to convert from usec to steps...
//
// (2048 phases/ 360 degrees) * (180 degrees / 1000 usec) = 1 phase per usec.
#define PhasePerUSec 1

#ifndef CONNECTORBOARD
volatile bool userPressed = false;
#endif

// This depends on all X pins being in the same bank.
#define X_MASK ( X_0_Pin | X_1_Pin | X_2_Pin | X_3_Pin )

// This depends on all Y pins being in the same bank.
#define Y0_MASK (Y0_0_Pin | Y0_1_Pin | Y0_2_Pin | Y0_3_Pin)
#define Y1_MASK (Y1_0_Pin | Y1_1_Pin | Y1_2_Pin | Y1_3_Pin)
#define Y_MASK (Y0_MASK | Y1_MASK)

// Declared in main.c
extern "C" TIM_HandleTypeDef htim14;
extern "C" TIM_HandleTypeDef htim16;
extern "C" I2C_HandleTypeDef hi2c1;
extern "C" void restoreGPIO( void );

// Global instance of our registers.
//
// Made global so the I2C interrupts and DMA can see it.

// One step of the motor is 4 phase activations.
//  This sequence does a full step and provides decent torque.
//
// These coil activations require that the coils for a particular
//  motor reside in the same bank.
//
// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t xCoils[] = { (X_0_Pin), // | (X_1_Pin),         // 0b1000,
		(X_0_Pin) | (X_1_Pin),     // 0b1100,
		(X_1_Pin), // | (X_2_Pin), // 0b0100,
		(X_1_Pin) | (X_2_Pin),     // 0b0110,
		(X_2_Pin), // | (X_3_Pin), // 0b0010,
		(X_2_Pin) | (X_3_Pin),     // 0b0011,
		(X_3_Pin), // | (X_0_Pin), // 0b0001,
		(X_3_Pin) | (X_0_Pin),
// 0b1001,
		};

// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t y0Coils[] = { (Y0_0_Pin), // | (Y0_1_Pin),
		(Y0_0_Pin) | (Y0_1_Pin), (Y0_1_Pin), // | (Y0_2_Pin),
		(Y0_1_Pin) | (Y0_2_Pin), (Y0_2_Pin), // | (Y0_3_Pin),
		(Y0_2_Pin) | (Y0_3_Pin), (Y0_3_Pin), // | (Y0_0_Pin),
		(Y0_3_Pin) | (Y0_0_Pin), };

const uint16_t y1Coils[] = { (Y1_0_Pin), // | (Y1_1_Pin),
		(Y1_0_Pin) | (Y1_1_Pin), (Y1_1_Pin), // | (Y1_2_Pin),
		(Y1_1_Pin) | (Y1_2_Pin), (Y1_2_Pin), // | (Y1_3_Pin),
		(Y1_2_Pin) | (Y1_3_Pin), (Y1_3_Pin), // | (Y1_0_Pin),
		(Y1_3_Pin) | (Y1_0_Pin), };

// 0x3 for full step or 0x7 for 1/2 step.
#define PhaseMask 0x3



MotorQueue xQueue(xCoils, X_0_GPIO_Port, X_MASK, false);
MotorQueue yQueue(y0Coils, Y0_0_GPIO_Port, Y0_MASK, false);
MotorQueue yPQueue(y1Coils, Y1_0_GPIO_Port, Y1_MASK, false);

void testPins( GPIO_TypeDef *port, uint16_t pins, uint16_t *record );

Registers *registers = &SystemRegisters;

extern "C" void mazeMain(void) {

	uint32_t now = HAL_GetTick();

	bool selfTestComplete = false;
	bool forward = true;

	int n = 0;

	HAL_TIM_Base_Start_IT(&htim14);

	htim14.Instance->ARR = 10000 - 1;

	if (registers->mode == Registers::Mode::PWM) {
		HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);
	}

	// Should guard this with interrupt off
	HAL_I2C_EnableListen_IT(&hi2c1);

	//HAL_Delay(1000);

	while (1) {

		uint8_t TxBuf = 0x21;  // Magic!  Turn the oscillator on
		const uint8_t displayAddr = 0x70 << 1;
		HAL_StatusTypeDef ret;

		// Update error counts
		registers->errCount = xQueue.errCount() + yQueue.errCount()
				+ yPQueue.errCount();

		if (registers->mode == Registers::Mode::Test &&
				registers->selfTest == Registers::SelfTestResult::NotRun) {

			// Assume we pass
			registers->selfTest = Registers::SelfTestResult::Pass;

			registers->yPins = 0;
			registers->xPins = 0;

			testPins( Y0_0_GPIO_Port, Y_MASK, &registers->yPins);
			testPins( X_0_GPIO_Port, X_MASK, &registers->xPins);

			if ( registers->xPins != X_MASK ){
				registers->selfTest = Registers::SelfTestResult::Fail;
				registers->failCode |= FAILCODE_PIN;
			}
			// Put the pins back the way they are supposed to be
			restoreGPIO();

			// See if we can write to the display
			ret = HAL_I2C_Master_Transmit(&hi2c1, displayAddr, &TxBuf, 1, 1000);
			if ( ret != HAL_StatusTypeDef::HAL_OK ){
				registers->failCode |= FAILCODE_I2CH;
				registers->selfTest = Registers::SelfTestResult::Fail;
			}
			restoreGPIO();

			if ( registers->selfTest == Registers::SelfTestResult::Pass) {
				bool worked = true;
				HT16K33 display( &hi2c1 );
				worked = display.turnOn(15);
				worked = worked && display.pass();
				if ( !worked ){
					registers->failCode = FAILCODE_DISP;
					registers->selfTest = Registers::SelfTestResult::Fail;
				}
			}

			// What we're actually doing
			if (false && xQueue.isEmpty() && yQueue.isEmpty() && yPQueue.isEmpty()) {
				if (!selfTestComplete) {
					if (n == 0) {
						n++;
						if (forward) {
							xQueue.push(PPR * 4);
						} else {
							xQueue.push(-PPR * 4);
						}
					}

					else if (n == 1) {
						n++;
						if (forward) {
							yQueue.push(PPR * 4);
						} else {
							yQueue.push(-PPR * 4);
						}
					}

					else if (n == 2) {
						n = 0;

						if (forward) {
							forward = !forward;
							yPQueue.push(PPR * 4);
						} else {
							yPQueue.push(-PPR * 4);
							selfTestComplete = true;
						}
					}

				} else {
					HAL_GPIO_WriteMultipleStatePin(X_0_GPIO_Port, X_MASK, 0);
					HAL_GPIO_WriteMultipleStatePin(Y0_0_GPIO_Port,
					Y0_MASK | Y1_MASK, 0);
				}
				HAL_GPIO_WriteMultipleStatePin(X_0_GPIO_Port, X_MASK, 0);
				HAL_GPIO_WriteMultipleStatePin(Y0_0_GPIO_Port, Y_MASK, 0);
			}
		}

		if (registers->mode == Registers::Mode::Calibrate) {
			if (HAL_GetTick() > now + 10000) {
				if (yQueue.isEmpty()) {
					HAL_TIM_Base_Stop_IT(&htim14);
					xQueue.idle();
					yQueue.idle();
					yPQueue.idle();
					HAL_Delay(1000);
					yQueue.push(4 * 2048);
					yPQueue.push(-4 * 2048);
					HAL_TIM_Base_Start_IT(&htim14);
				} else {
					now = HAL_GetTick();
				}
			}
		}
	}

}

void testPins( GPIO_TypeDef *port, uint16_t pinMask, uint16_t *record ){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	uint16_t currPin = 0x1;

	// What we should do.

	// This test is basically to make sure there are no bridging faults
	// around the processor.
	//
	// Make all the pins input with weak pulldown.  Drive a each pin high in
	// succession, reading and confirming between each pass.
	//
	// Go the other way.  Weak pullup, drive low.  Check.

	// Start with the Y1 and Y0 pins.
	GPIO_InitStruct.Pin = pinMask;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
	HAL_Delay(100);
	if ( (port->IDR & pinMask) != 0 ) {
		*record = ~0;
	} else {

		do {
			while ( currPin != 0 && (currPin & Y_MASK) == 0 ){ currPin <<= 1; }
			GPIO_InitStruct.Pin = currPin;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			HAL_GPIO_Init(port, &GPIO_InitStruct);
			HAL_Delay(100);
			if ((port->IDR & pinMask) == currPin ) {
				*record |= currPin;
			}

			// Put it back
			GPIO_InitStruct.Pin = currPin;
			GPIO_InitStruct.Pull = GPIO_PULLDOWN;
			HAL_GPIO_Init(port, &GPIO_InitStruct);
			HAL_Delay(100);
			currPin <<=1;

		} while ( currPin );
	}
}

extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {

#ifndef CONNECTORBOARD
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;
#endif

	if (GPIO_Pin == StepX_Pin && registers->mode == Registers::Mode::StepDir) {
		// Check the direction
		if (HAL_GPIO_ReadPin(DirX_GPIO_Port, DirX_Pin) == GPIO_PinState::GPIO_PIN_RESET) {
			xQueue.push(xQueue.halfStep() ? 4 : 2);
		} else {
			xQueue.push(xQueue.halfStep() ? -4 : -2);
		}
	}

	if (GPIO_Pin == StepY_Pin && registers->mode == Registers::Mode::StepDir) {

		if (HAL_GPIO_ReadPin(DirY_GPIO_Port, DirY_Pin) == GPIO_PinState::GPIO_PIN_RESET) {
			yQueue.push(yQueue.halfStep() ? 4 : 2);
			yPQueue.push(yQueue.halfStep() ? -4 : -2);
		} else {
			yQueue.push(yQueue.halfStep() ? -4 : -2);
			yPQueue.push(yQueue.halfStep() ? 4 : 2);
		}
	}

	if (GPIO_Pin == StepY_Pin && registers->mode == Registers::Mode::Calibrate) {
		if (HAL_GPIO_ReadPin(DirY_GPIO_Port, DirY_Pin)) {
			yQueue.push(1);
		} else {
			yPQueue.push(1);
		}
	}
}

// PWM mode mostly happens here.
extern "C" void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim16) {
		if (HAL_GPIO_ReadPin(StepX_GPIO_Port, StepX_Pin) == GPIO_PIN_SET) {
			// rising edge - 0 the counter
			htim->Instance->CNT = 0;
		} else {
			// falling edge, report out the value
			//targetXPW = htim->Instance->CNT;
		}
	}
}

// This ISR controls the motor coils.  The queues store the number of fractional
//  steps to take.  The ISR will pop the next value and work on it until it is
//  done.  There is no motion planning here...  If you want to move both motors
//  simultaneously, that needs to be coordinated in the outer loop.
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if (htim == &htim14) {
		xQueue.step();
		yQueue.step();
		yPQueue.step();
	}
}

extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
	HAL_I2C_EnableListen_IT(&hi2c1);
}



//--------- Documentation ------------------
//
// Basic timer setup.  This updates the stepper at 625Hz.
//
//htim14.Instance = TIM14;
//htim14.Init.Prescaler = 2-1;
//htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
//htim14.Init.Period = 38400-1;
//htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
//{
//  Error_Handler();
//}
