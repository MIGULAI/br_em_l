#include "relay.h"
#include "main.h"

#define RELAY_MIN_INTERVAL_MS  20U   /* захист від надто частого перемикання */
#define RELAY_PORT_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

static bool     relay_state = false;
static uint32_t last_switch_tick = 0;

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/* NO contact closes to GND on pickup: active (target reached) == LOW */
static bool relay_feedback_active(void)
{
    return HAL_GPIO_ReadPin(RELAY_FB_PORT, RELAY_FB_PIN) == GPIO_PIN_RESET;
}

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

    gpio.Pin  = RELAY_FB_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(RELAY_FB_PORT, &gpio);

    dwt_init();

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

uint32_t relay_measure_switch_delay_us(bool target_state, uint32_t timeout_ms)
{
    if (relay_state == target_state) {
        return 0;
    }
    if (!relay_set(target_state)) {
        return 0;
    }

    uint32_t start          = DWT->CYCCNT;
    uint32_t timeout_cycles = timeout_ms * (SystemCoreClock / 1000U);

    while (relay_feedback_active() != target_state) {
        if ((uint32_t)(DWT->CYCCNT - start) > timeout_cycles) {
            return UINT32_MAX;
        }
    }

    return (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
}
