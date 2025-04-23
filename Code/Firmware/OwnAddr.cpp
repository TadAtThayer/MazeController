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

Registers *registers = &SystemRegisters;

volatile uint8_t regAddr = 0xff;
volatile uint8_t rxDone = 0;
volatile uint8_t txDone = 0;

extern "C" I2C_HandleTypeDef hi2c1;

extern "C" void mazeMain(void) {
	uint8_t buf = 0x55;

	//HAL_I2C_Master_Transmit(&hi2c1, 0xe0, &buf, 1, 100);

	HAL_I2C_EnableListen_IT(&hi2c1);
	I2C1->CR1 = 0x9;

	while (1) {

		if ( rxDone || txDone ) {
			HAL_I2C_EnableListen_IT(&hi2c1);
			I2C1->CR1 = 0x9;
			rxDone = 0;
			txDone = 0;
		}


	}

}

extern "C" void I2C1_IRQHandler( void ) {
	I2C1->ICR = I2C_ICR_ADDRCF;
	if ( I2C1->ISR & I2C_ISR_DIR ){
		I2C1->TXDR = 1;
	} else {
		while(1);
	}
}

void do_nothing(void){
	while(1);
}

uint8_t rxBuffer[sizeof(Registers)] = {0};
uint8_t regNum;
uint8_t *txBuffer = (uint8_t *)registers;

extern "C" void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){
	if ( TransferDirection == I2C_DIRECTION_TRANSMIT ){
		HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, rxBuffer, sizeof(Registers), I2C_FIRST_AND_LAST_FRAME);
	} else {
		uint8_t addr = rxBuffer[0];
		uint8_t len = 1;
		HAL_I2C_Slave_Seq_Transmit_IT(&hi2c1, &(txBuffer[addr]), len, I2C_FIRST_AND_LAST_FRAME);
	}
}

extern "C" void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
	do_nothing();
}

extern "C" void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
	do_nothing();
}

extern "C" void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c){
	txDone = 1;
}

extern "C" void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){
	rxDone = 1;
}

//extern "C" void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
//	do_nothing();
//}

extern "C" void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c){
	do_nothing();
}

extern "C" void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){
	do_nothing();
}

extern "C" void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c){
	do_nothing();
}

// look at https://community.st.com/t5/stm32-mcus/how-to-create-an-i2c-slave-device-using-the-stm32cube-library/ta-p/49844

