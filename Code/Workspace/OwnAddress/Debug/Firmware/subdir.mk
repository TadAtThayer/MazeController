################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/Users/d66317d/Code/Boards/MazeController/Code/Firmware/MotorQueue.cpp \
/Users/d66317d/Code/Boards/MazeController/Code/Firmware/MoveQueue.cpp \
/Users/d66317d/Code/Boards/MazeController/Code/Firmware/OwnAddr.cpp \
/Users/d66317d/Code/Boards/MazeController/Code/Firmware/Registers.cpp 

OBJS += \
./Firmware/MotorQueue.o \
./Firmware/MoveQueue.o \
./Firmware/OwnAddr.o \
./Firmware/Registers.o 

CPP_DEPS += \
./Firmware/MotorQueue.d \
./Firmware/MoveQueue.d \
./Firmware/OwnAddr.d \
./Firmware/Registers.d 


# Each subdirectory must supply rules for building sources it contributes
Firmware/MotorQueue.o: /Users/d66317d/Code/Boards/MazeController/Code/Firmware/MotorQueue.cpp Firmware/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0plus -std=gnu++14 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32C031xx -c -I../Core/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32C0xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -I"/Users/d66317d/Code/Boards/MazeController/Code/Gen" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Firmware/MoveQueue.o: /Users/d66317d/Code/Boards/MazeController/Code/Firmware/MoveQueue.cpp Firmware/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0plus -std=gnu++14 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32C031xx -c -I../Core/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32C0xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -I"/Users/d66317d/Code/Boards/MazeController/Code/Gen" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Firmware/OwnAddr.o: /Users/d66317d/Code/Boards/MazeController/Code/Firmware/OwnAddr.cpp Firmware/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0plus -std=gnu++14 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32C031xx -c -I../Core/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32C0xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -I"/Users/d66317d/Code/Boards/MazeController/Code/Gen" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Firmware/Registers.o: /Users/d66317d/Code/Boards/MazeController/Code/Firmware/Registers.cpp Firmware/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0plus -std=gnu++14 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32C031xx -c -I../Core/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32C0xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -I"/Users/d66317d/Code/Boards/MazeController/Code/Gen" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Firmware

clean-Firmware:
	-$(RM) ./Firmware/MotorQueue.cyclo ./Firmware/MotorQueue.d ./Firmware/MotorQueue.o ./Firmware/MotorQueue.su ./Firmware/MoveQueue.cyclo ./Firmware/MoveQueue.d ./Firmware/MoveQueue.o ./Firmware/MoveQueue.su ./Firmware/OwnAddr.cyclo ./Firmware/OwnAddr.d ./Firmware/OwnAddr.o ./Firmware/OwnAddr.su ./Firmware/Registers.cyclo ./Firmware/Registers.d ./Firmware/Registers.o ./Firmware/Registers.su

.PHONY: clean-Firmware

