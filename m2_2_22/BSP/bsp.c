#include "bsp.h"
#include "main.h"   // HAL and CubeMX definitions
#include "usart.h"

void bsp_led_write(bool on)
{
    // PC13 LED is active low: RESET = on, SET = off
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void bsp_led_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

uint32_t bsp_millis(void)
{
    return HAL_GetTick();
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}