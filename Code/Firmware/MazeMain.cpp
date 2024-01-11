/*
 * MazeMain.cpp
 *
 *  Created on: Jan 9, 2024
 *      Author: d66317d
 */
#include "main.h"
#include <stdint.h>
#include <assert.h>

volatile bool userPressed = false;

// These are written by the ISR and read by the main loop.
volatile int8_t stepXCount = 0;
volatile int8_t stepYCount = 0;

void stepXY( int8_t xCount, int8_t yCount );

int8_t stepsToGo(int8_t stepsCompleted, int8_t totalStepsNeeded );

// Declared in main.c
extern "C" TIM_HandleTypeDef htim14;

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

extern "C" void mazeMain(void){
	uint8_t quadrant = 0;

	HAL_TIM_Base_Start_IT(&htim14);

	HAL_GPIO_WritePin(Y1_2_GPIO_Port, Y1_2_Pin, GPIO_PinState::GPIO_PIN_RESET);
	HAL_Delay(1000);

	while(1){
		for ( unsigned i = 0; i < 2032; i++) {
			while(xQueue.full()){
				HAL_GPIO_WritePin(Y1_2_GPIO_Port, Y1_2_Pin, GPIO_PinState::GPIO_PIN_SET);
			}
			//HAL_GPIO_WritePin(Y1_2_GPIO_Port, Y1_2_Pin, GPIO_PinState::GPIO_PIN_RESET);
			xQueue.push(activeCoils[i&0x3]);
		}
		HAL_Delay(5000);
	}

}


extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;
}



extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if ( ! xQueue.isEmpty() ){
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
