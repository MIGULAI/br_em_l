#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BTN GPIO_NUM_4
#define LED GPIO_NUM_5

void app_main(void)
{
    gpio_config_t led = {
        .pin_bit_mask = 1ULL << LED,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led);

    gpio_config_t btn = {
        .pin_bit_mask = 1ULL << BTN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&btn);

    while (1) {
        gpio_set_level(LED, !gpio_get_level(BTN));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}