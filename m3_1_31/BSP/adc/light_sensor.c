#include "light_sensor.h"
#include "bsp.h"


adc_status_t light_sensor_init(light_sensor_config_t config, struct light_sensor_s *self) {
    self->config = config;
    self->hadc = config.hadc;
    self->last_reading = 0;

    // Initialize the ADC channel here if needed
    // For example, configure the ADC channel, resolution, and sampling time

    return ADC_OK;
}

void light_sensor_read(light_sensor_t *sensor) {
    // Start ADC conversion
    HAL_ADC_Start(sensor->hadc);

    // Wait for conversion to complete
    if (HAL_ADC_PollForConversion(sensor->hadc, HAL_MAX_DELAY) == HAL_OK) {
        // Read the ADC value
        sensor->last_reading = HAL_ADC_GetValue(sensor->hadc);
    }

    // Stop ADC conversion
    HAL_ADC_Stop(sensor->hadc);
}

bool light_sensor_is_ready(light_sensor_t *sensor) {
    uint32_t current_time = HAL_GetTick();
    if (current_time - sensor->last_read_time_ms >= 100) { // 100 ms delay
        sensor->last_read_time_ms = current_time;
        return true;
    }
    return false;
}

float light_sensor_get_voltage(light_sensor_t *sensor, float reference_voltage) {
    return (sensor->last_reading * reference_voltage) / 4095.0f;
}
