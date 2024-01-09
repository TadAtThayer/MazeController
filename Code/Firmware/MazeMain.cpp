/*
 * MazeMain.cpp
 *
 *  Created on: Jan 9, 2024
 *      Author: d66317d
 */
#include "main.h"


volatile bool userPressed = false;


extern "C" void mazeMain(void){

	while(1){
		if ( userPressed ){
			userPressed = false;
			HAL_GPIO_TogglePin( Y1_2_GPIO_Port, Y1_2_Pin);

		}
	}

}



extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if ( GPIO_Pin == User_Button_Pin ) userPressed = true;
}
