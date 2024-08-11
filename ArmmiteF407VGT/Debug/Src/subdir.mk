################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Audio.c \
../Src/BmpDecoder.c \
../Src/CFunctions.c \
../Src/Commands.c \
../Src/Custom.c \
../Src/Draw.c \
../Src/Editor.c \
../Src/External.c \
../Src/FileIO.c \
../Src/Flash.c \
../Src/Functions.c \
../Src/GPS.c \
../Src/GUI.c \
../Src/I2C.c \
../Src/Keyboard.c \
../Src/MATHS.c \
../Src/MMBasic.c \
../Src/MM_Custom.c \
../Src/MM_Misc.c \
../Src/Memory.c \
../Src/MiscSTM32.c \
../Src/Onewire.c \
../Src/Operators.c \
../Src/PWM.c \
../Src/SPI-LCD.c \
../Src/SPI.c \
../Src/SSD1963.c \
../Src/Serial.c \
../Src/SerialFileIO.c \
../Src/Timers.c \
../Src/Touch.c \
../Src/XModem.c \
../Src/bsp_driver_sd.c \
../Src/cJSON.c \
../Src/fatfs.c \
../Src/fatfs_platform.c \
../Src/main.c \
../Src/sd_diskio.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/stm32f4xx_ll_gpio.c \
../Src/stm32f4xx_ll_spi.c \
../Src/syscalls.c \
../Src/system_stm32f4xx.c \
../Src/usb_device.c \
../Src/usbd_cdc_if.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c 

OBJS += \
./Src/Audio.o \
./Src/BmpDecoder.o \
./Src/CFunctions.o \
./Src/Commands.o \
./Src/Custom.o \
./Src/Draw.o \
./Src/Editor.o \
./Src/External.o \
./Src/FileIO.o \
./Src/Flash.o \
./Src/Functions.o \
./Src/GPS.o \
./Src/GUI.o \
./Src/I2C.o \
./Src/Keyboard.o \
./Src/MATHS.o \
./Src/MMBasic.o \
./Src/MM_Custom.o \
./Src/MM_Misc.o \
./Src/Memory.o \
./Src/MiscSTM32.o \
./Src/Onewire.o \
./Src/Operators.o \
./Src/PWM.o \
./Src/SPI-LCD.o \
./Src/SPI.o \
./Src/SSD1963.o \
./Src/Serial.o \
./Src/SerialFileIO.o \
./Src/Timers.o \
./Src/Touch.o \
./Src/XModem.o \
./Src/bsp_driver_sd.o \
./Src/cJSON.o \
./Src/fatfs.o \
./Src/fatfs_platform.o \
./Src/main.o \
./Src/sd_diskio.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/stm32f4xx_ll_gpio.o \
./Src/stm32f4xx_ll_spi.o \
./Src/syscalls.o \
./Src/system_stm32f4xx.o \
./Src/usb_device.o \
./Src/usbd_cdc_if.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o 

C_DEPS += \
./Src/Audio.d \
./Src/BmpDecoder.d \
./Src/CFunctions.d \
./Src/Commands.d \
./Src/Custom.d \
./Src/Draw.d \
./Src/Editor.d \
./Src/External.d \
./Src/FileIO.d \
./Src/Flash.d \
./Src/Functions.d \
./Src/GPS.d \
./Src/GUI.d \
./Src/I2C.d \
./Src/Keyboard.d \
./Src/MATHS.d \
./Src/MMBasic.d \
./Src/MM_Custom.d \
./Src/MM_Misc.d \
./Src/Memory.d \
./Src/MiscSTM32.d \
./Src/Onewire.d \
./Src/Operators.d \
./Src/PWM.d \
./Src/SPI-LCD.d \
./Src/SPI.d \
./Src/SSD1963.d \
./Src/Serial.d \
./Src/SerialFileIO.d \
./Src/Timers.d \
./Src/Touch.d \
./Src/XModem.d \
./Src/bsp_driver_sd.d \
./Src/cJSON.d \
./Src/fatfs.d \
./Src/fatfs_platform.d \
./Src/main.d \
./Src/sd_diskio.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/stm32f4xx_ll_gpio.d \
./Src/stm32f4xx_ll_spi.d \
./Src/syscalls.d \
./Src/system_stm32f4xx.d \
./Src/usb_device.d \
./Src/usbd_cdc_if.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/Commands.o: ../Src/Commands.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -O2 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/Functions.o: ../Src/Functions.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/MMBasic.o: ../Src/MMBasic.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -O2 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/Operators.o: ../Src/Operators.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/Timers.o: ../Src/Timers.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 '-D__weak=__attribute__((weak))' -DSTM32F4version -DSTM32 '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Inc -I../Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FatFs/src -O2 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/Audio.d ./Src/Audio.o ./Src/Audio.su ./Src/BmpDecoder.d ./Src/BmpDecoder.o ./Src/BmpDecoder.su ./Src/CFunctions.d ./Src/CFunctions.o ./Src/CFunctions.su ./Src/Commands.d ./Src/Commands.o ./Src/Commands.su ./Src/Custom.d ./Src/Custom.o ./Src/Custom.su ./Src/Draw.d ./Src/Draw.o ./Src/Draw.su ./Src/Editor.d ./Src/Editor.o ./Src/Editor.su ./Src/External.d ./Src/External.o ./Src/External.su ./Src/FileIO.d ./Src/FileIO.o ./Src/FileIO.su ./Src/Flash.d ./Src/Flash.o ./Src/Flash.su ./Src/Functions.d ./Src/Functions.o ./Src/Functions.su ./Src/GPS.d ./Src/GPS.o ./Src/GPS.su ./Src/GUI.d ./Src/GUI.o ./Src/GUI.su ./Src/I2C.d ./Src/I2C.o ./Src/I2C.su ./Src/Keyboard.d ./Src/Keyboard.o ./Src/Keyboard.su ./Src/MATHS.d ./Src/MATHS.o ./Src/MATHS.su ./Src/MMBasic.d ./Src/MMBasic.o ./Src/MMBasic.su ./Src/MM_Custom.d ./Src/MM_Custom.o ./Src/MM_Custom.su ./Src/MM_Misc.d ./Src/MM_Misc.o ./Src/MM_Misc.su ./Src/Memory.d ./Src/Memory.o ./Src/Memory.su ./Src/MiscSTM32.d ./Src/MiscSTM32.o ./Src/MiscSTM32.su ./Src/Onewire.d ./Src/Onewire.o ./Src/Onewire.su ./Src/Operators.d ./Src/Operators.o ./Src/Operators.su ./Src/PWM.d ./Src/PWM.o ./Src/PWM.su ./Src/SPI-LCD.d ./Src/SPI-LCD.o ./Src/SPI-LCD.su ./Src/SPI.d ./Src/SPI.o ./Src/SPI.su ./Src/SSD1963.d ./Src/SSD1963.o ./Src/SSD1963.su ./Src/Serial.d ./Src/Serial.o ./Src/Serial.su ./Src/SerialFileIO.d ./Src/SerialFileIO.o ./Src/SerialFileIO.su ./Src/Timers.d ./Src/Timers.o ./Src/Timers.su ./Src/Touch.d ./Src/Touch.o ./Src/Touch.su ./Src/XModem.d ./Src/XModem.o ./Src/XModem.su ./Src/bsp_driver_sd.d ./Src/bsp_driver_sd.o ./Src/bsp_driver_sd.su ./Src/cJSON.d ./Src/cJSON.o ./Src/cJSON.su ./Src/fatfs.d ./Src/fatfs.o ./Src/fatfs.su ./Src/fatfs_platform.d ./Src/fatfs_platform.o ./Src/fatfs_platform.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/sd_diskio.d ./Src/sd_diskio.o ./Src/sd_diskio.su ./Src/stm32f4xx_hal_msp.d ./Src/stm32f4xx_hal_msp.o ./Src/stm32f4xx_hal_msp.su ./Src/stm32f4xx_it.d ./Src/stm32f4xx_it.o ./Src/stm32f4xx_it.su ./Src/stm32f4xx_ll_gpio.d ./Src/stm32f4xx_ll_gpio.o ./Src/stm32f4xx_ll_gpio.su ./Src/stm32f4xx_ll_spi.d ./Src/stm32f4xx_ll_spi.o ./Src/stm32f4xx_ll_spi.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/system_stm32f4xx.d ./Src/system_stm32f4xx.o ./Src/system_stm32f4xx.su ./Src/usb_device.d ./Src/usb_device.o ./Src/usb_device.su ./Src/usbd_cdc_if.d ./Src/usbd_cdc_if.o ./Src/usbd_cdc_if.su ./Src/usbd_conf.d ./Src/usbd_conf.o ./Src/usbd_conf.su ./Src/usbd_desc.d ./Src/usbd_desc.o ./Src/usbd_desc.su

.PHONY: clean-Src

