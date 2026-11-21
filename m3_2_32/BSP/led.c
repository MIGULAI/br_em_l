#include "led.h"
#include "bsp.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdio.h>

led_status_t led_init(led_config_t led_config, struct led_s *self) {
  if (!gpio_clk_enable(led_config.port)) {
    return LED_ERROR; // Failed to enable GPIO clock
  }

  self->config = led_config;
  self->state = false;
  self->last_change_ms = 0;

  HAL_GPIO_WritePin(self->config.port, (1 << self->config.pin),
                    self->config.active_high ? GPIO_PIN_RESET : GPIO_PIN_SET);

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = (1 << led_config.pin);
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(led_config.port, &GPIO_InitStruct);
  self->last_change_ms = HAL_GetTick();
  return LED_OK;
}


void led_toggle(struct led_s *led) {
    uint32_t current_time = HAL_GetTick();
    if (current_time - led->last_change_ms >= led->config.change_delay_ms) {
        led->state = !led->state;
        GPIO_PinState new_state = (led->state == led->config.active_high) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(led->config.port, (1 << led->config.pin),
                        new_state);
        led->last_change_ms = current_time;
    }
}