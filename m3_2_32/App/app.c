#include "app.h"
#include "led.h"
#include "usart.h"
#include "potentiometer.h"
#include <stdint.h>
#include <stdio.h>

struct led_s board_led_instance;
struct potentiometer_s potentiometer_instance;


led_config_t led_config = {
    .pin = 13, .port = GPIOC, .active_high = true, .change_delay_ms = 500};

potentiometer_config_t potentiometer_config = {
    .channel = ADC_CHANNEL_1, .resolution = ADC_RESOLUTION_12B, .sampling_time = ADC_SAMPLETIME_3CYCLES, .hadc = &hadc1};

void app_init(void) {
     led_status_t board_led = led_init(led_config, &board_led_instance);
  if (board_led != LED_OK) {
    printf("LED initialization failed\n");
  } else {
    printf("LED initialized successfully\n");
  }
    adc_status_t potentiometer_status = potentiometer_init(potentiometer_config, &potentiometer_instance);
  if (potentiometer_status != ADC_OK) {
    printf("Potentiometer initialization failed\n");
  } else {
    printf("Potentiometer initialized successfully\n");  
  }
}

void app_run(void) {
    // Main application loop here
    led_toggle(&board_led_instance); 
        if (potentiometer_is_ready(&potentiometer_instance)) {
        potentiometer_read(&potentiometer_instance);
        uint32_t mv = potentiometer_instance.last_reading * 3300U / 4095U;
        uint32_t voltage = potentiometer_get_voltage(&potentiometer_instance, 3.3f); // Already in mV
        printf("Potentiometer raw: %lu, calibrated voltage: %lu mV, uncalibrated voltage: %lu mV\r\n", potentiometer_instance.last_reading, voltage, mv);
    }
}

int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}