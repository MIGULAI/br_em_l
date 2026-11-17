#include "app.h"
#include "led.h"
#include "servo.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

struct servo_s servo;
struct led_s board_led_instance;

servo_config_t servo_config = {.tim = TIM1,
                               .channel = 1,
                               .port = GPIOA,
                               .pin = 8,
                               .af = 1,
                               .min_pulse_width = 500,
                               .max_pulse_width = 2400,
                               .max_angle_deg = 180};

led_config_t led_config = {
    .pin = 13, .port = GPIOC, .active_high = true, .change_delay_ms = 500};


void app_init(void) {
  // Initialization code here
  led_status_t board_led = led_init(led_config, &board_led_instance);
  if (board_led != LED_OK) {
    printf("LED initialization failed\n");
  } else {
    printf("LED initialized successfully\n");
  }

  if (servo_init(servo_config, &servo) != SERVO_OK) {
    printf("Servo initialization failed\n");
  } else {
    printf("Servo initialized successfully\n");
  }
  printf("Board Initialised\n");
  servo_set_angle(&servo, 90); // Set servo to 90 degrees
  HAL_Delay(1000);    // Wait for 1 second
  servo_set_angle(&servo, 0);  // Set servo to 0 degrees
}

void app_run(void) { led_toggle(&board_led_instance); }

int _write(int file, char *ptr, int len) {
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
