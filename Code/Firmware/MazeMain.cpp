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



volatile bool userPressed = false;
volatile bool xCommand = false;
volatile bool yCommand = false;



// These are written by the ISR and read by the main loop.
volatile int8_t xStepCount = 0;
volatile int8_t yStepCount = 0;

void stepXY( int8_t xCount, int8_t yCount );

int8_t stepsToGo(int8_t stepsCompleted, int8_t totalStepsNeeded );

// Declared in main.c
extern "C" TIM_HandleTypeDef htim14;
extern "C" TIM_HandleTypeDef htim16;

// One step of the motor is 4 phase acitvations.
//  This sequence does a full step and provides decent torque.
//
// The coil activity will be stored in a queue.  The edge triggered
//  "Step" inputs will add to the queue.  The timer interript will
// remove from the queue and drive the coils.
const uint8_t activeCoils[] = {
		0b1100,
		0b0110,
		0b0011,
		0b1001
};

enum class Mode : uint8_t {
	StepDir = 0,
	PWM = 1,
	Calibrate
};

struct {
	Mode mode = Mode::StepDir;
	uint8_t errCount = 0;
} registers;


class MoveQueue {
public:
	static const unsigned int size = 2048;
private:
	uint8_t q[MoveQueue::size>>1];
	unsigned int wrEntry = 0;
	unsigned int rdEntry = 0;
	bool wrapped = false;

	// Helper functions
	uint8_t *wrPtr() { return &q[wrEntry>>1]; }
	uint8_t *rdPtr() { return &q[rdEntry>>1]; }

	bool wrOdd() { return wrEntry & 0x1 ? true : false; }
	bool rdOdd() { return rdEntry & 0x1 ? true : false; }

public :
	bool isEmpty(){ return count() == 0; }
	unsigned int count(){ return (wrapped ? size : 0 ) + wrEntry - rdEntry; }
	bool full(){ return count() == size; }


	// Push the low 4 bits into the appropriate place.
	// Not worried about overflow as we need to size this
	// to be large enough to keep up with the incoming
	// requests.
	void push( uint8_t item ){
		item &= 0xf;
		if (wrOdd()){
			item <<= 4;
			*wrPtr() &= 0x0f;
		} else {
			*wrPtr() &= 0xf0;
		}
		*wrPtr() |= item;
		// Bump pointer and wrap
		wrEntry++;
		if ( wrEntry == size ) wrapped = true;
		wrEntry &= (size - 1);
	}

	// Pop the top element.
	//  If we are empty, just return whatever is there.
	//  Consumer must check full/empty status.
	uint8_t pop() {
		uint8_t val = *rdPtr();

		// justify the actual bits
		val = rdOdd() ? val >> 4 : val & 0x0f;
		if ( count() > 0  ) {
			// bump and wrap
			rdEntry++;
			if ( rdEntry == size ) wrapped = false;
			rdEntry &= (size - 1);
		}
		return val;
	}


};

MoveQueue xQueue;
MoveQueue yQueue;

volatile 	uint16_t targetXPW = 1500;
volatile 	uint16_t targetYPW = 1500;

extern "C" void mazeMain(void){
	uint8_t xQuadrant = 0;
	uint8_t yQuadrant = 0;

	uint16_t currentXPW = 1500;
	uint16_t currentYPW = 1500;

	bool forward = true;
	uint8_t stepCount;

	HAL_TIM_Base_Start_IT(&htim14);
	HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);
	// Should guard this with interrupt off

	while(1) {

		if ( currentXPW != targetXPW ){
			// figure out how many pulses to issue
			int increment = 1;
			int phases = (currentXPW - targetXPW) * PhasePerUSec;
			if ( phases < 0 ){
				phases = -phases;
				increment = -1;
			}
			for ( int i = 0; i < phases; i++ ){
				xQueue.push(activeCoils[xQuadrant]);
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
				yQueue.push(activeCoils[yQuadrant]);
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
				xQueue.push(activeCoils[xQuadrant]);
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
				yQueue.push(activeCoils[yQuadrant]);
				yQuadrant += (forward ? 1 : -1); yQuadrant &= 0x3;
			}
			yCommand = false;
		}
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
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;

	if ( GPIO_Pin == StepX_Pin && registers.mode == Mode::StepDir ){
		// Check the direction
		if ( HAL_GPIO_ReadPin(DirX_GPIO_Port, DirX_Pin) ){
			xStepCount = -1;
		} else {
			xStepCount = 1;
		}
		xCommand = true;
	}

	if ( GPIO_Pin == StepY_Pin && registers.mode == Mode::StepDir ){
		if ( HAL_GPIO_ReadPin(DirY_GPIO_Port, DirY_Pin) ) {
			yStepCount = 1;
		} else {
			yStepCount = -1;
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

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if ( htim == &htim16 ) {
		while(1);
	}

	if ( htim == &htim14 && !xQueue.isEmpty() ){
		// Drive the motors here.
		uint8_t nextCoilX = xQueue.pop();

		// drive the X coils.  Note: This assumes that the controller is running fast
		//  enough that the motors won't notice the fact that the coils are not changing
		//  simultaneously.  If they do, we'll need to use the BSBR register.
		HAL_GPIO_WritePin(X_0_GPIO_Port, X_0_Pin, nextCoilX & 0x1 ? GPIO_PinState::GPIO_PIN_SET : GPIO_PinState::GPIO_PIN_RESET);
		HAL_GPIO_WritePin(X_1_GPIO_Port, X_1_Pin, nextCoilX & 0x2 ? GPIO_PinState::GPIO_PIN_SET : GPIO_PinState::GPIO_PIN_RESET);
		HAL_GPIO_WritePin(X_2_GPIO_Port, X_2_Pin, nextCoilX & 0x4 ? GPIO_PinState::GPIO_PIN_SET : GPIO_PinState::GPIO_PIN_RESET);
		HAL_GPIO_WritePin(X_3_GPIO_Port, X_3_Pin, nextCoilX & 0x8 ? GPIO_PinState::GPIO_PIN_SET : GPIO_PinState::GPIO_PIN_RESET);
	}
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
