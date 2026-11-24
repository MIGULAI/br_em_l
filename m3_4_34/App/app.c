#include "app.h"
#include "led.h"
#include "usart.h"
#include "tim.h"
#include "buzzer.h"
#include <stdint.h>
#include <stdio.h>

struct led_s board_led_instance;

static const note_t fx_beep_up[] = { {1000,80}, {0,30}, {1500,80}, {0,30}, {2000,150} };
static const note_t fx_error[]   = { {400,200}, {0,50}, {300,300} };

static const note_t fx_imperial_march[] = {
    // Phrase 1
    {NOTE_G4, 480}, GAP, {NOTE_G4, 480}, GAP, {NOTE_G4, 480}, GAP,
    {NOTE_DS4, 330}, GAP, {NOTE_AS4, 130}, GAP,
    {NOTE_G4, 480}, GAP,
    {NOTE_DS4, 330}, GAP, {NOTE_AS4, 130}, GAP,
    {NOTE_G4, 980}, {0, 500},

    // Phrase 2
    {NOTE_D5, 480}, GAP, {NOTE_D5, 480}, GAP, {NOTE_D5, 480}, GAP,
    {NOTE_DS5, 330}, GAP, {NOTE_AS4, 130}, GAP,
    {NOTE_FS4, 480}, GAP,
    {NOTE_DS4, 330}, GAP, {NOTE_AS4, 130}, GAP,
    {NOTE_G4, 1000},
};

led_config_t led_config = {
    .pin = 13, .port = GPIOC, .active_high = true, .change_delay_ms = 500};

void app_init(void)
{
  led_status_t board_led = led_init(led_config, &board_led_instance);
  if (board_led != LED_OK)
  {
    printf("LED initialization failed\n");
  }
  else
  {
    printf("LED initialized successfully\n");
  }

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  buzzer_play(fx_imperial_march, sizeof fx_imperial_march / sizeof fx_beep_up[0]); 
  // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 250); // Set initial duty cycle to 50%
}

void app_run(void)
{
  // Main application loop here
  led_toggle(&board_led_instance);
  buzzer_update();  

}

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}