################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/freertos.c \
../Src/gcode.c \
../Src/gpio.c \
../Src/main.c \
../Src/serial.c \
../Src/shell-cmds.c \
../Src/shell-core.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_hal_timebase_TIM.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/usart.c 

OBJS += \
./Src/freertos.o \
./Src/gcode.o \
./Src/gpio.o \
./Src/main.o \
./Src/serial.o \
./Src/shell-cmds.o \
./Src/shell-core.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_hal_timebase_TIM.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/usart.o 

C_DEPS += \
./Src/freertos.d \
./Src/gcode.d \
./Src/gpio.d \
./Src/main.d \
./Src/serial.d \
./Src/shell-cmds.d \
./Src/shell-core.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_hal_timebase_TIM.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"E:/drive/code/stm32/shell/Inc" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI" -I"E:/drive/code/stm32/shell/Drivers/STM32F4xx_HAL_Driver/Inc" -I"E:/drive/code/stm32/shell/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"E:/drive/code/stm32/shell/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/include" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"E:/drive/code/stm32/shell/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/shell-core.o: ../Src/shell-core.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/microrl/src" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI" -I"E:/drive/code/stm32/shell/Inc" -I"E:/drive/code/stm32/shell/Drivers/STM32F4xx_HAL_Driver/Inc" -I"E:/drive/code/stm32/shell/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"E:/drive/code/stm32/shell/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/include" -I"E:/drive/code/stm32/shell/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"E:/drive/code/stm32/shell/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


