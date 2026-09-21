#include "app.h"
#include "button.h"
#include "usart.h"
#include <stdbool.h>
#include <stdio.h>

static button_handler_t self;

void app_init(void) {
    button_config_t button_config = {
        .pin = GPIO_PIN_0,
        .port = GPIOB,
        .active_low = true          // Example active low configuration
    };
    self = button_init(button_config);
    printf("App is running.\n");
}

void app_run(void) {
    button_process(self);
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}