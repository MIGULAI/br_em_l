#pragma once

#include "adc.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum { ADC_OK = 1, ADC_ERROR } adc_status_t;

typedef struct {
  uint32_t channel;
  uint32_t resolution;
  uint32_t sampling_time;
  ADC_HandleTypeDef *hadc; // Pointer to the ADC handle
} light_sensor_config_t;

typedef struct light_sensor_s {
  light_sensor_config_t config;
  ADC_HandleTypeDef *hadc;
  uint32_t last_reading;
  uint32_t last_read_time_ms;
} light_sensor_t;

adc_status_t light_sensor_init(light_sensor_config_t config, struct light_sensor_s *self);
void light_sensor_read(light_sensor_t *sensor);
bool light_sensor_is_ready(light_sensor_t *sensor);

float light_sensor_get_voltage(light_sensor_t *sensor, float reference_voltage);