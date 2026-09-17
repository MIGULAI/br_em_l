#ifndef RELAY_H
#define RELAY_H

#include <stdbool.h>
#include <stdint.h>

#define RELAY_PIN       GPIO_PIN_12
#define RELAY_PORT GPIOB

/* NO-contact feedback: closes to GND on pickup, pulled up when idle */
#define RELAY_FB_PIN    GPIO_PIN_13
#define RELAY_FB_PORT   GPIOB

void relay_init(void);
bool relay_set(bool on);
bool relay_toggle(void);
bool relay_is_on(void);

/*
 * Commands the coil to target_state and blocks until the NO-contact
 * feedback on RELAY_FB_PIN confirms it, returning the elapsed time in
 * microseconds (mechanical pickup/release delay).
 *
 * Returns 0 if relay is already in target_state (nothing to measure) or
 * if relay_set() was blocked by the min-switch-interval guard.
 * Returns UINT32_MAX if the feedback contact never confirms within
 * timeout_ms.
 */
uint32_t relay_measure_switch_delay_us(bool target_state, uint32_t timeout_ms);

#endif