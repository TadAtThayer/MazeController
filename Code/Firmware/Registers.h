/*
 * Registers.h
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <stdint.h>

#include "gitver.h"

// Define some bits for the failure code.
#define FAILCODE_NONE 0x0
#define FAILCODE_PIN  0x1
#define FAILCODE_I2CH 0x2   // Host transaction failure
#define FAILCODE_DISP 0x4


class Registers {

public:
	enum SelfTestResult {
		Fail = 0, Pass = 1, NotRun
	};

	enum Mode {
		StepDir = 0, PWM = 1, Test = 2, Calibrate
	};

	uint8_t major = 0;
	uint8_t minor = 0;
	uint16_t gitver = GITVER;
	Mode mode = Mode::Calibrate;
	uint8_t errCount = 0;
	SelfTestResult selfTest = SelfTestResult::NotRun;
	uint16_t failCode = 0;
	uint16_t offTime = 5000;  // ms
	uint16_t xPins = 0;
	uint16_t yPins = 0;
};

extern Registers SystemRegisters;

#endif /* REGISTERS_H_ */
