#pragma once

#include "adc.h"
#include <stdbool.h>
#include <stdint.h>

#define Vref 1210

typedef enum { ADC_OK = 1, ADC_ERROR } adc_status_t;


typedef struct {
  uint32_t channel;
  uint32_t resolution;
  uint32_t sampling_time;
  ADC_HandleTypeDef *hadc; // Pointer to the ADC handle
} potentiometer_config_t;

typedef struct potentiometer_s {
  potentiometer_config_t config;
  ADC_HandleTypeDef *hadc;
  uint32_t last_reading;
  uint32_t last_read_time_ms;
  uint16_t vref_real;
  float k_calc;
} potentiometer_t;

adc_status_t potentiometer_init(potentiometer_config_t config, struct potentiometer_s *self);
void potentiometer_read(potentiometer_t *potentiometer);
bool potentiometer_is_ready(potentiometer_t *potentiometer);

uint32_t potentiometer_get_voltage(potentiometer_t *potentiometer, float reference_voltage);