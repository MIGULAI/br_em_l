#include "relay.h"
#include "main.h"

#define RELAY_MIN_INTERVAL_MS  20U   /* захист від надто частого перемикання */
#define RELAY_PORT_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

static bool     relay_state = false;
static uint32_t last_switch_tick = 0;

void relay_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RELAY_PORT_CLK_ENABLE();

    /* Set output latch LOW before enabling the output: no click at startup */
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);

    gpio.Pin   = RELAY_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RELAY_PORT, &gpio);

    relay_state      = false;
    last_switch_tick = HAL_GetTick();
}

bool relay_set(bool on ) {
    if (on == relay_state) {
        return true; 
    }
    if ((uint32_t)(HAL_GetTick() - last_switch_tick) < RELAY_MIN_INTERVAL_MS) {
        return false; 
    }
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    relay_state = on;
    last_switch_tick = HAL_GetTick();
    return true;
}

bool relay_toggle(void) {
    return relay_set(!relay_state);
}

bool relay_is_on(void) {
    return relay_state;
}
