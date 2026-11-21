#include "stm32f4xx_ll_adc.h"
#include "potentiometer.h"
#include "bsp.h"
#include <stdio.h>


adc_status_t potentiometer_init(potentiometer_config_t config, struct potentiometer_s *self) {
    self->config = config;
    self->hadc = config.hadc;
    self->last_reading = 0;

    // get vref_real from calibration data
    uint16_t *vref_cal = (uint16_t *)VREFINT_CAL_ADDR;
    self->vref_real = *vref_cal * 3300 / 4095; 
    self->k_calc = self->vref_real / Vref; // Calculate the scaling factor
    printf("Potentiometer vref_real: %u\r\n", self->vref_real);
    return ADC_OK;
}

void potentiometer_read(potentiometer_t *potentiometer) {
    // Start ADC conversion
    HAL_ADC_Start(potentiometer->hadc);

    // Wait for conversion to complete
    if (HAL_ADC_PollForConversion(potentiometer->hadc, HAL_MAX_DELAY) == HAL_OK) {
        // Read the ADC value
        potentiometer->last_reading = HAL_ADC_GetValue(potentiometer->hadc);
    }

    // Stop ADC conversion
    HAL_ADC_Stop(potentiometer->hadc);
}

bool potentiometer_is_ready(potentiometer_t *potentiometer) {
    uint32_t current_time = HAL_GetTick();
    if (current_time - potentiometer->last_read_time_ms >= 100) { // 100 ms delay
        potentiometer->last_read_time_ms = current_time;
        return true;
    }
    return false;
}

uint32_t potentiometer_get_voltage(potentiometer_t *potentiometer, float reference_voltage) {
    return (uint32_t)(((potentiometer->last_reading * reference_voltage) / 4095.0f * potentiometer->k_calc) * 1000); // Convert to mV
}