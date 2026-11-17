#pragma once
#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum { SERVO_OK = 1, SERVO_ERROR } servo_status_t;

typedef struct {
  TIM_TypeDef *tim;
  uint8_t channel;
  GPIO_TypeDef *port; // pin wired to that timer channel
  uint8_t pin;
  uint8_t af;
  uint16_t min_pulse_width;
  uint16_t max_pulse_width;
  uint16_t max_angle_deg;
} servo_config_t;

struct servo_s {
  servo_config_t config;
  volatile uint32_t *ccr; // points to TIMx->CCR1..CCR4 for this channel
  uint16_t current_angle;
};

servo_status_t servo_init(servo_config_t servo_config, struct servo_s *self);
void servo_set_pulse_width(struct servo_s *servo, uint16_t pulse_width);
void servo_set_angle(struct servo_s *servo, uint16_t angle);
