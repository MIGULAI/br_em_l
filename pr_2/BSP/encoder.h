#pragma once
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

#define ENCODER_COUNTER_PERIOD  0xFFFFU
#define ENCODER_FILTER_MAX      0x0FU


typedef struct {
  uint8_t pin;
  GPIO_TypeDef *port;
#if !defined(STM32F1)
  uint8_t alternate;
#endif
} encoder_pin_config_t;

typedef struct {
  TIM_TypeDef *tim_instance;
  encoder_pin_config_t ch_a_config;
  encoder_pin_config_t ch_b_config;
  uint8_t counts_per_detent;
  uint8_t input_filter;
  bool invert_direction;
} encoder_config_t;

typedef struct {
  encoder_config_t config;
  TIM_HandleTypeDef tim_handle;
  int16_t last_count;
  int32_t residual;
} encoder_t;

typedef enum {
  ENCODER_OK = 0,
  ENCODER_ERR_PARAM,
  ENCODER_ERR_HAL
} encoder_status_t;

encoder_status_t encoder_init(encoder_t *enc, encoder_config_t *config);

int32_t encoder_get_step(encoder_t *encoder);

void encoder_reset(encoder_t *encoder);