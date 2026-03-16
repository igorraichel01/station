################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/acquisition.c \
../Core/Src/bmp180.c \
../Core/Src/data_buffer.c \
../Core/Src/dht11.c \
../Core/Src/ds3231.c \
../Core/Src/i18n.c \
../Core/Src/lcd_i2c.c \
../Core/Src/main.c \
../Core/Src/menu.c \
../Core/Src/pad20.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/usb_datalogger.c \
../Core/Src/w25qxx.c 

OBJS += \
./Core/Src/acquisition.o \
./Core/Src/bmp180.o \
./Core/Src/data_buffer.o \
./Core/Src/dht11.o \
./Core/Src/ds3231.o \
./Core/Src/i18n.o \
./Core/Src/lcd_i2c.o \
./Core/Src/main.o \
./Core/Src/menu.o \
./Core/Src/pad20.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/usb_datalogger.o \
./Core/Src/w25qxx.o 

C_DEPS += \
./Core/Src/acquisition.d \
./Core/Src/bmp180.d \
./Core/Src/data_buffer.d \
./Core/Src/dht11.d \
./Core/Src/ds3231.d \
./Core/Src/i18n.d \
./Core/Src/lcd_i2c.d \
./Core/Src/main.d \
./Core/Src/menu.d \
./Core/Src/pad20.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/usb_datalogger.d \
./Core/Src/w25qxx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/acquisition.cyclo ./Core/Src/acquisition.d ./Core/Src/acquisition.o ./Core/Src/acquisition.su ./Core/Src/bmp180.cyclo ./Core/Src/bmp180.d ./Core/Src/bmp180.o ./Core/Src/bmp180.su ./Core/Src/data_buffer.cyclo ./Core/Src/data_buffer.d ./Core/Src/data_buffer.o ./Core/Src/data_buffer.su ./Core/Src/dht11.cyclo ./Core/Src/dht11.d ./Core/Src/dht11.o ./Core/Src/dht11.su ./Core/Src/ds3231.cyclo ./Core/Src/ds3231.d ./Core/Src/ds3231.o ./Core/Src/ds3231.su ./Core/Src/i18n.cyclo ./Core/Src/i18n.d ./Core/Src/i18n.o ./Core/Src/i18n.su ./Core/Src/lcd_i2c.cyclo ./Core/Src/lcd_i2c.d ./Core/Src/lcd_i2c.o ./Core/Src/lcd_i2c.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/menu.cyclo ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/menu.su ./Core/Src/pad20.cyclo ./Core/Src/pad20.d ./Core/Src/pad20.o ./Core/Src/pad20.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/usb_datalogger.cyclo ./Core/Src/usb_datalogger.d ./Core/Src/usb_datalogger.o ./Core/Src/usb_datalogger.su ./Core/Src/w25qxx.cyclo ./Core/Src/w25qxx.d ./Core/Src/w25qxx.o ./Core/Src/w25qxx.su

.PHONY: clean-Core-2f-Src

