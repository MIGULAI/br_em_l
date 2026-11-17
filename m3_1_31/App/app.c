#include "app.h"
#include "led.h"
#include "light_sensor.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

struct led_s board_led_instance;
struct light_sensor_s light_sensor_instance;

led_config_t led_config = {
    .pin = 13, .port = GPIOC, .active_high = true, .change_delay_ms = 500};

light_sensor_config_t light_sensor_config = {
    .channel = ADC_CHANNEL_1, .resolution = ADC_RESOLUTION_12B, .sampling_time = ADC_SAMPLETIME_3CYCLES, .hadc = &hadc1};

void app_init(void) {
     led_status_t board_led = led_init(led_config, &board_led_instance);
  if (board_led != LED_OK) {
    printf("LED initialization failed\n");
  } else {
    printf("LED initialized successfully\n");
  }
  adc_status_t light_sensor = light_sensor_init(light_sensor_config, &light_sensor_instance);
  if (light_sensor != ADC_OK) {
    printf("Light sensor initialization failed\n");
  } else {
    printf("Light sensor initialized successfully\n");  
  }
}

void app_run(void) {
    // Main application loop here
    led_toggle(&board_led_instance); 
    if (light_sensor_is_ready(&light_sensor_instance)) {
        light_sensor_read(&light_sensor_instance);
        uint32_t mv = light_sensor_instance.last_reading * 3300U / 4095U;
        printf("Light sensor raw: %lu, voltage: %lu mV\r\n", light_sensor_instance.last_reading, mv);
    }
}

int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}