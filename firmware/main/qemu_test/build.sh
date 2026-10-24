set -e

FLAGS="-mcpu=cortex-m3 -mthumb -Os -g \
  -DUSE_HAL_DRIVER -DSTM32F103xB \
  -IInc \
  -IDrivers/STM32F1xx_HAL_Driver/Inc \
  -IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
  -IDrivers/CMSIS/Include \
  -IDrivers/UART/Inc"

HAL=Drivers/STM32F1xx_HAL_Driver/Src
OUT=qemu_test/out
mkdir -p $OUT

for f in Src/system_stm32f1xx.c \
         $HAL/stm32f1xx_hal.c $HAL/stm32f1xx_hal_cortex.c $HAL/stm32f1xx_hal_rcc.c \
         $HAL/stm32f1xx_hal_gpio.c $HAL/stm32f1xx_hal_uart.c $HAL/stm32f1xx_hal_dma.c; do
    arm-none-eabi-gcc $FLAGS -c $f -o $OUT/$(basename $f .c).o
done
for f in qemu_test/test_main.cpp Drivers/UART/Src/uart_transmit.cpp; do
    arm-none-eabi-g++ $FLAGS -std=gnu++14 -fno-rtti -fno-exceptions -c $f -o $OUT/$(basename $f .cpp).o
done

arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -c startup_stm32f103xb.s -o $OUT/startup.o

# 4) link everything with the 8K-RAM linker script
arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -specs=nano.specs -specs=nosys.specs \
  -Wl,--gc-sections -T qemu_test/qemu.ld $OUT/*.o -o $OUT/uart_test.elf

arm-none-eabi-size $OUT/uart_test.elf