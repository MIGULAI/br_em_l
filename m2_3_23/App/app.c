#include "app.h"
#include "led.h"
#include "usart.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


led_handler_t led1;
led_handler_t led2;
led_handler_t led3;

void app_init(void) {
    // PB3
    led_config_t led1_config = {
        .pin = GPIO_PIN_3,
        .port = GPIOB,
        .active_high = true,
        .change_delay_ms = 200,
    };
        led_config_t led2_config = {
        .pin = GPIO_PIN_4,
        .port = GPIOB,
        .active_high = true,
        .change_delay_ms = 500,
    };
        led_config_t led3_config = {
        .pin = GPIO_PIN_5,
        .port = GPIOB,
        .active_high = true,
        .change_delay_ms = 1000,
    };
    led1 = led_init(led1_config);
    led2 = led_init(led2_config);
    led3 = led_init(led3_config);
    printf("App initialized.\n");
}

void app_run(void) {
    printf("App is running.\n");
    while (1) {
        if (led_should_toggle(led1)) {
            led_toggle(led1);
        }
        if (led_should_toggle(led2)) {
            led_toggle(led2);
        }
        if (led_should_toggle(led3)) {
            led_toggle(led3);
        }
        // Application logic here
    }
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}