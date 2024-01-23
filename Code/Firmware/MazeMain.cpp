/*
 * MazeMain.cpp
 *
 *  Created on: Jan 9, 2024
 *      Author: d66317d
 */
#include "main.h"
#include <stdint.h>
#include <assert.h>


// These are the number of phase activations per 360 degress.
#define PPR 2032

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
// (2032 phases/ 360 degrees) * (180 degrees / 1000 usec) = 1 phase per usec.
#define PhasePerUSec 1


#ifndef CONNECTORBOARD
volatile bool userPressed = false;
#endif

volatile bool xCommand = false;
volatile bool yCommand = false;

// This depends on all X pins being in the same bank.
#define X_MASK ( X_0_Pin | X_1_Pin | X_2_Pin | X_3_Pin )

// This depends on all Y pins being in the same bank.
#define Y0_MASK (Y0_0_Pin | Y0_1_Pin | Y0_2_Pin | Y0_3_Pin)
#define Y1_MASK (Y1_0_Pin | Y1_1_Pin | Y1_2_Pin | Y1_3_Pin)


// Declared in main.c
extern "C" TIM_HandleTypeDef htim14;
extern "C" TIM_HandleTypeDef htim16;
extern "C" I2C_HandleTypeDef hi2c1;

// One step of the motor is 4 phase acitvations.
//  This sequence does a full step and provides decent torque.
//
// These coil activations require that the coils for a particular
//  motor reside in the same bank.
//
// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t xCoils[] = {
		(X_0_Pin),// | (X_1_Pin), // 0b1100,
		(X_1_Pin),// | (X_2_Pin), // 0b0110,
		(X_2_Pin),// | (X_3_Pin), // 0b0011,
		(X_3_Pin),// | (X_0_Pin), // 0b1001
};

// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t y0Coils[] = {
		(Y0_0_Pin),// | (Y0_1_Pin), // 0b1100,
		(Y0_1_Pin),// | (Y0_2_Pin), // 0b0110,
		(Y0_2_Pin),// | (Y0_3_Pin), // 0b0011,
		(Y0_3_Pin),// | (Y0_0_Pin), // 0b1001
};

const uint16_t y1Coils[] = {
		(Y1_0_Pin), // | (Y1_1_Pin), // 0b1100,
		(Y1_1_Pin), // | (Y1_2_Pin), // 0b0110,
		(Y1_2_Pin), // | (Y1_3_Pin), // 0b0011,
		(Y1_3_Pin), // | (Y1_0_Pin), // 0b1001
};

// 0x3 for full step or 0x7 for 1/2 step.
#define PhaseMask 0x3


enum class Mode : uint8_t {
	StepDir = 0,
	PWM = 1,
	Test = 2,
	Calibrate
};

enum class SelfTestResult : uint8_t {
	Fail = 0,
	Pass = 1,
	NotRun
};

struct {
	Mode mode = Mode::Test;
	uint8_t errCount = 0;
	SelfTestResult selfTest = SelfTestResult::NotRun;
} registers;


class MoveQueue {
public:
	static const unsigned int size = 128;
private:
	int16_t q[MoveQueue::size];
	unsigned int wrEntry = 0;
	unsigned int rdEntry = 0;
	bool wrapped = false;

	// Helper functions
	int16_t *wrPtr() { return &q[wrEntry]; }
	int16_t *rdPtr() { return &q[rdEntry]; }

public :
	bool isEmpty(){ return count() == 0; }
	unsigned int count(){ return (wrapped ? size : 0 ) + wrEntry - rdEntry; }
	bool full(){ return count() == size; }


	// Push the low 4 bits into the appropriate place.
	// Not worried about overflow as we need to size this
	// to be large enough to keep up with the incoming
	// requests.
	void push( int16_t item ){
		*wrPtr() = item;
		// Bump pointer and wrap
		wrEntry++;
		if ( wrEntry == size ) wrapped = true;
		wrEntry &= (size - 1);
	}

	// Pop the top element.
	//  If we are empty, just return whatever is there.
	//  Consumer must check full/empty status.
	int16_t pop() {
		int16_t val = *rdPtr();

		if ( count() > 0  ) {
			// bump and wrap
			rdEntry++;
			if ( rdEntry == size ) wrapped = false;
			rdEntry &= (size - 1);
		}
		return val;
	}

	// Look at the head element
	int16_t peek() {
		int16_t val = *rdPtr();
		return val;
	}


};

MoveQueue xQueue;
MoveQueue yQueue;
MoveQueue yPQueue;

volatile 	uint16_t targetXPW = 1500;
volatile 	uint16_t targetYPW = 1500;

extern "C" void mazeMain(void){

	bool selfTestComplete = false;
	bool forward = true;

	int n = 0;

	HAL_TIM_Base_Start_IT(&htim14);


	if ( registers.mode == Mode::PWM ) {
		HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);
	}

	// Should guard this with interrupt off
	HAL_I2C_EnableListen_IT(&hi2c1);


	while(1) {

		if ( registers.mode == Mode::Test && !selfTestComplete ){
			// Do the next tick of self testing.

			// What we should do.

			// This test is basically to make sure there are no bridging faults
			// around the processor.
			//
			// Make all the pins input with weak pulldown.  Drive a each pin high in
			// succession, reading and confirming between each pass.
			//
			// Go the other way.  Weak pullup, drive low.  Check.
			//
			// Return to default.

			// What we're actually doing
			if ( xQueue.isEmpty() && yQueue.isEmpty() && yPQueue.isEmpty() ){
				if ( n == 0) {
					n++;
					if ( forward ) {
						xQueue.push(2048);
					} else {
						xQueue.push(-2048);
					}
				}

				else if ( n == 1 ) {
					n++;
					if ( forward ){
						yQueue.push(2048);
					} else {
						yQueue.push(-2048);
					}
				}

				else if ( n == 2 ) {
					n = 0;
					forward = !forward;

					if ( forward ){
						yPQueue.push(2048);
					} else {
						yPQueue.push(-2048);
					}
				}
			}


		}
/*
		if ( currentXPW != targetXPW ){
			// figure out how many pulses to issue
			int increment = 1;
			int phases = (currentXPW - targetXPW) * PhasePerUSec;
			if ( phases < 0 ){
				phases = -phases;
				increment = -1;
			}
			for ( int i = 0; i < phases; i++ ){
				xQueue.push(xCoils[xQuadrant]);
				xQuadrant += increment; xQuadrant &= 0x3;
			}
			currentXPW = targetXPW;
		}

		if ( currentYPW != targetYPW ){
			// figure out how many pulses to issue
			int increment = 1;
			int phases = (currentYPW - targetYPW) * PhasePerUSec;
			if ( phases < 0 ){
				phases = -phases;
				increment = -1;
			}
			for ( int i = 0; i < phases; i++ ){
				yQueue.push(xCoils[yQuadrant]);
				yQuadrant += increment; yQuadrant &= 0x3;
			}
			currentYPW = targetYPW;
		}
		if ( xCommand ){ // signal from the interrupt handler
			forward = xStepCount >= 0;
			if ( forward ){
				stepCount = xStepCount;
			} else {
				stepCount = -xStepCount;
			}
			for ( int i = 0; i < stepCount << 2; i++ ){
				// Add 4 quarter steps per step
				xQueue.push(xCoils[xQuadrant]);
				xQuadrant += (forward ? 1 : -1); xQuadrant &= 0x3;
			}
			xCommand = false;
		}

		if ( yCommand ){
			forward = yStepCount >= 0;
			if ( forward ) {
				stepCount = yStepCount;
			} else {
				stepCount = -yStepCount;
			}
			for ( int i = 0; i < stepCount << 2; i++ ){
				yQueue.push(xCoils[yQuadrant]);
				yQuadrant += (forward ? 1 : -1); yQuadrant &= 0x3;
			}
			yCommand = false;
		}
		*/
	}

//	while(1){
//		for ( unsigned i = 0; i < 2032; i++) {
//			while(xQueue.full()){
//				HAL_GPIO_WritePin(Y1_2_GPIO_Port, Y1_2_Pin, GPIO_PinState::GPIO_PIN_SET);
//			}
//			//HAL_GPIO_WritePin(Y1_2_GPIO_Port, Y1_2_Pin, GPIO_PinState::GPIO_PIN_RESET);
//			xQueue.push(activeCoils[i&0x3]);
//		}
//		HAL_Delay(5000);
//	}

}


extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){

#ifndef CONNECTORBOARD
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;
#endif

	if ( GPIO_Pin == StepX_Pin && registers.mode == Mode::StepDir ){
		// Check the direction
		if ( HAL_GPIO_ReadPin(DirX_GPIO_Port, DirX_Pin) ){
			//xStepCount = -1;
		} else {
			//xStepCount = 1;
		}
		xCommand = true;
	}

	if ( GPIO_Pin == StepY_Pin && registers.mode == Mode::StepDir ){
		if ( HAL_GPIO_ReadPin(DirY_GPIO_Port, DirY_Pin) ) {
			//yStepCount = 1;
		} else {
			//yStepCount = -1;
		}
		yCommand = true;
	}

}

extern "C" void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim ){
	if ( htim == &htim16 ) {
		if ( HAL_GPIO_ReadPin(StepX_GPIO_Port, StepX_Pin) == GPIO_PIN_SET) {
			// rising edge - 0 the counter
			htim->Instance->CNT = 0;
		} else {
			// falling edge, report out the value
			targetXPW = htim->Instance->CNT;
		}
	}
}

// This ISR controls the motor coils.  The queues store the number of fractional
//  steps to take.  The ISR will pop the next value and work on it until it is
//  done.  There is no motion planning here...  If you want to move both motors
//  simultaneously, that needs to be coordinated in the outer loop.
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	static int xStepsTaken = 0;
	static int yStepsTaken = 0;


	if ( htim == &htim14) {

		if ( !xQueue.isEmpty() ){
			if ( xStepsTaken != xQueue.peek() ){
				// drive the motors.
				uint8_t nextCoilX = xCoils[xStepsTaken & PhaseMask];

				if ( xQueue.peek() > 0 ){
					xStepsTaken++;
				} else {
					xStepsTaken--;
				}

				// This takes advantage of the fact that the BSR register is set priority,
				//  so the pins that are both reset and set, end up getting set.
				HAL_GPIO_WriteMultipleStatePin(X_0_GPIO_Port, X_MASK, nextCoilX);
			}

			if ( xStepsTaken == xQueue.peek() ){
				xQueue.pop();
				xStepsTaken = 0;
			}
		}

		if ( yStepsTaken == yStepsToTake && !yQueue.isEmpty() ){
			yStepsToTake = yQueue.pop();
			yStepsTaken = 0;
		}

		// Drive the motors here.
		if ( xStepsToTake ) {
		}

		// Drive the motors here.
		if ( yStepsToTake ) {
			uint16_t nextCoilY0 = y0Coils[yStepsTaken & PhaseMask];
			uint16_t nextCoilY1 = y1Coils[yStepsTaken & PhaseMask];

			if ( yStepsToTake > 0 ){
				yStepsTaken++;
			} else {
				yStepsTaken--;
			}

			// This takes advantage of the fact that the BSR register is set priority,
			//  so the pins that are both reset and set, end up getting set.
			HAL_GPIO_WriteMultipleStatePin(Y0_0_GPIO_Port, Y0_MASK, nextCoilY0);
			HAL_GPIO_WriteMultipleStatePin(Y1_0_GPIO_Port, Y1_MASK, nextCoilY1);
		}
	}
}

extern "C" void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c){
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
