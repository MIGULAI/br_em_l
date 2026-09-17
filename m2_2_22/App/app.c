#include "app.h"
#include "bsp.h"
#include "relay.h"
#include <stdio.h>
#include <stdint.h>

#define RELAY_MIN_INTERVAL_MS 1000u

static uint32_t relay_tick;

void app_init(void)
{
    // bsp_led_write(false);
    relay_tick = bsp_millis();
    relay_init();
    printf("boot ok, tick=%lu\n", relay_tick);
}

void app_run(void)
{
    if ((uint32_t)(bsp_millis() - relay_tick) >= RELAY_MIN_INTERVAL_MS) {
        relay_tick += RELAY_MIN_INTERVAL_MS;

        bool     target = !relay_is_on();
        uint32_t delay_us = relay_measure_switch_delay_us(target, 100U);

        if (delay_us == UINT32_MAX) {
            printf("relay=%d switch_delay=TIMEOUT\n", target);
        } else {
            printf("relay=%d switch_delay_us=%lu\n", target, delay_us);
        }
    }
}