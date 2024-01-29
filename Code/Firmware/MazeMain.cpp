/*
 * MazeMain.cpp
 *
 *  Created on: Jan 9, 2024
 *      Author: d66317d
 */
#include "main.h"
#include <stdint.h>
#include <assert.h>


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


// Declared in main.c
extern "C" TIM_HandleTypeDef htim14;
extern "C" TIM_HandleTypeDef htim16;
extern "C" I2C_HandleTypeDef hi2c1;

// One step of the motor is 4 phase activations.
//  This sequence does a full step and provides decent torque.
//
// These coil activations require that the coils for a particular
//  motor reside in the same bank.
//
// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t xCoils[] = {
		(X_0_Pin) | (X_1_Pin), // 0b1100,
		(X_1_Pin) | (X_2_Pin), // 0b0110,
		(X_2_Pin) | (X_3_Pin), // 0b0011,
		(X_3_Pin) | (X_0_Pin), // 0b1001
};

// Ideally Y and Y' will reside in a single bank, but probably not
//  critical.
const uint16_t y0Coils[] = {
		(Y0_0_Pin) | (Y0_1_Pin), // 0b1100,
		(Y0_1_Pin) | (Y0_2_Pin), // 0b0110,
		(Y0_2_Pin) | (Y0_3_Pin), // 0b0011,
		(Y0_3_Pin) | (Y0_0_Pin), // 0b1001
};

const uint16_t y1Coils[] = {
		(Y1_0_Pin) | (Y1_1_Pin), // 0b1100,
		(Y1_1_Pin) | (Y1_2_Pin), // 0b0110,
		(Y1_2_Pin) | (Y1_3_Pin), // 0b0011,
		(Y1_3_Pin) | (Y1_0_Pin), // 0b1001
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
	Mode mode = Mode::StepDir;
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
	uint8_t pushErrors = 0;
	uint8_t popErrors = 0;
	bool wrapped = false;

public :
	bool isEmpty(){ return count() == 0; }
	unsigned int count(){ return (wrapped ? size : 0 ) + wrEntry - rdEntry; }
	bool isFull(){ return count() == size; }


	void push( int16_t item ){
		if ( !isFull() ){
			q[wrEntry++] = item;
			if ( wrEntry == size ) wrapped = true;
			wrEntry &= (size - 1);
		} else {
			pushErrors++;
		}
	}

	// Pop the top element.
	//  If we are empty, just return whatever is there.
	//  Consumer must check full/empty status.
	int16_t pop() {
		int16_t val = q[rdEntry];

		if ( !isEmpty() ){
			if ( count() > 0  ) {
				// bump and wrap
				rdEntry++;
				if ( rdEntry == size ) wrapped = false;
				rdEntry &= (size - 1);
			}
		} else {
			popErrors++;
		}
		return val;
	}

	// Look at the head element
	int16_t peek() {
		int16_t val = q[rdEntry];
		return val;
	}

	uint8_t errCount() {
		return popErrors + pushErrors;
	}
};

class MotorQueue : public MoveQueue {
private :
	const uint32_t idleTime = 10000;    // ms.
	const uint16_t *coilActivations;
	bool fullStep = true;
	uint16_t pinMask;
	GPIO_TypeDef *gpioPort;
	int stepsTaken = 0;
	int quadrant = 0;
	uint32_t lastMoveTime = 0;

public :
	MotorQueue( const uint16_t *coils, GPIO_TypeDef *port, uint16_t pins ){
		coilActivations = coils;
		pinMask = pins;
		gpioPort = port;
	}

	void idle(){
		// Turn the coils off
		HAL_GPIO_WriteMultipleStatePin(gpioPort, pinMask, 0);
	}

	bool halfStep() { return !fullStep; }

	void step(void){
		int incVal = 1;

		if ( !isEmpty() ){
			lastMoveTime = HAL_GetTick();
			if ( stepsTaken > peek() ) {
				// We're going backward.
				incVal = -1;
			}

			// Drive the motors
			HAL_GPIO_WriteMultipleStatePin(gpioPort, pinMask, coilActivations[quadrant & (fullStep ? 0x3 : 0x7)]);
			quadrant += incVal;
			stepsTaken += incVal;

			if ( stepsTaken == peek() ){
				// We're done with the move at the head of the queue
				//  pop it and start the next.
				pop();
				stepsTaken = 0;
			}

		} else {
			if ( HAL_GetTick() - lastMoveTime > idleTime ){
				lastMoveTime = HAL_GetTick();
				idle();
			}
		}
	}

};

MotorQueue xQueue( xCoils, X_0_GPIO_Port, X_MASK );
MotorQueue yQueue( y0Coils, Y0_0_GPIO_Port, Y0_MASK );
MotorQueue yPQueue( y1Coils, Y1_0_GPIO_Port, Y1_MASK );


extern "C" void mazeMain(void){

	bool selfTestComplete = false;
	bool forward = true;

	int n = 0;

	HAL_TIM_Base_Start_IT(&htim14);

	htim14.Instance->ARR = 10000-1;

	if ( registers.mode == Mode::PWM ) {
		HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);
	}

	// Should guard this with interrupt off
	HAL_I2C_EnableListen_IT(&hi2c1);

	HAL_Delay(1000);

	while(1) {

		// Update error counts
		registers.errCount = xQueue.errCount() + yQueue.errCount() + yPQueue.errCount();

		if ( registers.mode == Mode::Test ) {

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
				if ( !selfTestComplete ){
					if ( n == 0) {
						n++;
						if ( forward ) {
							xQueue.push(PPR*4);
						} else {
							xQueue.push(-PPR*4);
						}
					}

					else if ( n == 1 ) {
						n++;
						if ( forward ){
							yQueue.push(PPR*4);
						} else {
							yQueue.push(-PPR*4);
						}
					}

					else if ( n == 2 ) {
						n = 0;

						if ( forward ){
							forward = !forward;
							yPQueue.push(PPR*4);
						} else {
							yPQueue.push(-PPR*4);
							selfTestComplete = true;
						}
					}

				} else {
					HAL_GPIO_WriteMultipleStatePin(X_0_GPIO_Port, X_MASK, 0);
					HAL_GPIO_WriteMultipleStatePin(Y0_0_GPIO_Port, Y0_MASK|Y1_MASK, 0);
				}
			}
		}
	}

}


extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){

#ifndef CONNECTORBOARD
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;
#endif

	if ( GPIO_Pin == StepX_Pin && registers.mode == Mode::StepDir ){
		// Check the direction
		if ( HAL_GPIO_ReadPin(DirX_GPIO_Port, DirX_Pin) ){
			xQueue.push( xQueue.halfStep() ? -8 : -4 );
		} else {
			xQueue.push(xQueue.halfStep() ? 8 : 4 );
		}
	}

	if ( GPIO_Pin == StepY_Pin && registers.mode == Mode::StepDir ){
		if ( HAL_GPIO_ReadPin(DirY_GPIO_Port, DirY_Pin) ) {
			yQueue.push( yQueue.halfStep() ? -8 : -4 );
			yPQueue.push( yQueue.halfStep() ? 8 : 4 );
		} else {
			yQueue.push( yQueue.halfStep() ? 8 : 4 );
			yPQueue.push( yQueue.halfStep() ? -8 : -4 );
		}
	}

}

// PWM mode mostly happens here.
extern "C" void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim ){
	if ( htim == &htim16 ) {
		if ( HAL_GPIO_ReadPin(StepX_GPIO_Port, StepX_Pin) == GPIO_PIN_SET) {
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
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if ( htim == &htim14) {
		xQueue.step();
		yQueue.step();
		yPQueue.step();

//		if ( !xQueue.isEmpty() ){
//			if ( xStepsTaken != xQueue.peek() ){
//				// drive the motors.
//				uint8_t nextCoilX = xCoils[xStepsTaken & PhaseMask];
//
//				if ( xQueue.peek() > 0 ){
//					xStepsTaken++;
//				} else {
//					xStepsTaken--;
//				}
//
//				// This takes advantage of the fact that the BSR register is set priority,
//				//  so the pins that are both reset and set, end up getting set.
//				HAL_GPIO_WriteMultipleStatePin(X_0_GPIO_Port, X_MASK, nextCoilX);
//			}
//
//			if ( xStepsTaken == xQueue.peek() ){
//				xQueue.pop();
//				xStepsTaken = 0;
//			}
//		}


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
