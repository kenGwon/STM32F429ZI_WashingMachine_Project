################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/DHT11.c \
../Core/Src/I2C_lcd.c \
../Core/Src/button.c \
../Core/Src/buzzer.c \
../Core/Src/dcmotor.c \
../Core/Src/fan_machine.c \
../Core/Src/fnd4digit.c \
../Core/Src/internal_rtc.c \
../Core/Src/ledbar.c \
../Core/Src/main.c \
../Core/Src/servomotor.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/timer_management.c \
../Core/Src/uart_management.c \
../Core/Src/ultrasonic.c \
../Core/Src/washing_machine.c \
../Core/Src/washing_machine2_deprecated.c 

OBJS += \
./Core/Src/DHT11.o \
./Core/Src/I2C_lcd.o \
./Core/Src/button.o \
./Core/Src/buzzer.o \
./Core/Src/dcmotor.o \
./Core/Src/fan_machine.o \
./Core/Src/fnd4digit.o \
./Core/Src/internal_rtc.o \
./Core/Src/ledbar.o \
./Core/Src/main.o \
./Core/Src/servomotor.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/timer_management.o \
./Core/Src/uart_management.o \
./Core/Src/ultrasonic.o \
./Core/Src/washing_machine.o \
./Core/Src/washing_machine2_deprecated.o 

C_DEPS += \
./Core/Src/DHT11.d \
./Core/Src/I2C_lcd.d \
./Core/Src/button.d \
./Core/Src/buzzer.d \
./Core/Src/dcmotor.d \
./Core/Src/fan_machine.d \
./Core/Src/fnd4digit.d \
./Core/Src/internal_rtc.d \
./Core/Src/ledbar.d \
./Core/Src/main.d \
./Core/Src/servomotor.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/timer_management.d \
./Core/Src/uart_management.d \
./Core/Src/ultrasonic.d \
./Core/Src/washing_machine.d \
./Core/Src/washing_machine2_deprecated.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/DHT11.cyclo ./Core/Src/DHT11.d ./Core/Src/DHT11.o ./Core/Src/DHT11.su ./Core/Src/I2C_lcd.cyclo ./Core/Src/I2C_lcd.d ./Core/Src/I2C_lcd.o ./Core/Src/I2C_lcd.su ./Core/Src/button.cyclo ./Core/Src/button.d ./Core/Src/button.o ./Core/Src/button.su ./Core/Src/buzzer.cyclo ./Core/Src/buzzer.d ./Core/Src/buzzer.o ./Core/Src/buzzer.su ./Core/Src/dcmotor.cyclo ./Core/Src/dcmotor.d ./Core/Src/dcmotor.o ./Core/Src/dcmotor.su ./Core/Src/fan_machine.cyclo ./Core/Src/fan_machine.d ./Core/Src/fan_machine.o ./Core/Src/fan_machine.su ./Core/Src/fnd4digit.cyclo ./Core/Src/fnd4digit.d ./Core/Src/fnd4digit.o ./Core/Src/fnd4digit.su ./Core/Src/internal_rtc.cyclo ./Core/Src/internal_rtc.d ./Core/Src/internal_rtc.o ./Core/Src/internal_rtc.su ./Core/Src/ledbar.cyclo ./Core/Src/ledbar.d ./Core/Src/ledbar.o ./Core/Src/ledbar.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/servomotor.cyclo ./Core/Src/servomotor.d ./Core/Src/servomotor.o ./Core/Src/servomotor.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/timer_management.cyclo ./Core/Src/timer_management.d ./Core/Src/timer_management.o ./Core/Src/timer_management.su ./Core/Src/uart_management.cyclo ./Core/Src/uart_management.d ./Core/Src/uart_management.o ./Core/Src/uart_management.su ./Core/Src/ultrasonic.cyclo ./Core/Src/ultrasonic.d ./Core/Src/ultrasonic.o ./Core/Src/ultrasonic.su ./Core/Src/washing_machine.cyclo ./Core/Src/washing_machine.d ./Core/Src/washing_machine.o ./Core/Src/washing_machine.su ./Core/Src/washing_machine2_deprecated.cyclo ./Core/Src/washing_machine2_deprecated.d ./Core/Src/washing_machine2_deprecated.o ./Core/Src/washing_machine2_deprecated.su

.PHONY: clean-Core-2f-Src

