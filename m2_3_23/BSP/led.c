#include "led.h"
#include <stdlib.h>

struct led_s {
    led_config_t config;
    bool state;
    uint32_t last_change_ms;
};

static struct led_s led_pool[LED_MAX_COUNT];
static uint8_t led_count;

static bool gpio_clk_enable(GPIO_TypeDef *port)
{
    if (port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); return true; }
#ifdef GPIOB
    if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOC
    if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOD
    if (port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOE
    if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); return true; }
#endif
#ifdef GPIOH
    if (port == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); return true; }
#endif
    return false;
}

static inline GPIO_PinState level_for(const struct led_s *self, bool on)
{
    return (on == self->config.active_high) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}


led_handler_t led_init(led_config_t led_config) {
    if (led_count >= LED_MAX_COUNT || led_config.pin == 0U) {
        return NULL;
    }
    if (!gpio_clk_enable(led_config.port)) {
        return NULL;
    }

    struct led_s *self = &led_pool[led_count];
    self->config = led_config;
    self->state = false;

    HAL_GPIO_WritePin(led_config.port, led_config.pin, led_config.active_high ? GPIO_PIN_RESET : GPIO_PIN_SET);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = led_config.pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(led_config.port, &gpio);
    self->last_change_ms = HAL_GetTick();

    led_count++;
    return self;
}

void led_set(led_handler_t self, bool on)
{
    if (self == NULL) {
        return;
    }
    HAL_GPIO_WritePin(self->config.port, self->config.pin, level_for(self, on));
    self->state = on;
    self->last_change_ms = HAL_GetTick();
}

void led_on(led_handler_t self)     { led_set(self, true); }
void led_off(led_handler_t self)    { led_set(self, false); }

void led_toggle(led_handler_t self)
{
    if (self == NULL) {
        return;
    }
    led_set(self, !self->state);
}

bool led_is_on(led_handler_t self)
{
    return (self != NULL) && self->state;
}

bool led_should_toggle(led_handler_t self)
{
    return (self != NULL) &&
           (self->last_change_ms + self->config.change_delay_ms <= HAL_GetTick());
}