#pragma once

#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum { LED_OK = 1, LED_ERROR } led_status_t;

typedef struct {
  uint8_t pin;
  GPIO_TypeDef *port;
  bool active_high;
  uint32_t change_delay_ms;
} led_config_t;

struct led_s {
  led_config_t config;
  bool state;
  uint32_t last_change_ms;
};

led_status_t led_init(led_config_t led_config, struct led_s *self);

void led_toggle(struct led_s *self);