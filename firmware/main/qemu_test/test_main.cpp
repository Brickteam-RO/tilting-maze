#include "stm32f1xx_hal.h"
#include "uart_transmit.h"

extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
}

int main() {
    HAL_Init();

    UART::UART_DEVICE dev(GPIOA, GPIO_PIN_9, GPIOA, GPIO_PIN_10);
    bool ok = dev.init();

    dev.println(ok ? "init OK" : "init FAILED");
    dev.println("hello from QEMU");

    while (1) {}
}