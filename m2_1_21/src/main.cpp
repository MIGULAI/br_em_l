#include <array>

#include "button.hpp"
#include "led.hpp"
#include "esp_log.h"
#include "app_config.hpp"

namespace
{

    app::Button g_button;
    app::Led g_led;

    StaticQueue_t g_q_ctrl;
    std::array<std::uint8_t, 8 * sizeof(app::ButtonEvent)> g_q_storage;

    constexpr const char *TAG = "app";

}

extern "C" void app_main()
{
    QueueHandle_t q = xQueueCreateStatic(8, sizeof(app::ButtonEvent),
                                         g_q_storage.data(), &g_q_ctrl);

    ESP_ERROR_CHECK(g_led.init({.gpio = APP_LED_GPIO, .active_low = false, .blink_period_ms = APP_LED_BLINK_PERIOD_MS}));
    ESP_ERROR_CHECK(g_button.init({
        .gpio = APP_BUTTON_GPIO,
        .debounce_ms = APP_BUTTON_DEBOUNCE_MS,
        .long_press_ms = APP_LONG_PRESS_MS,
        .out_queue = q,
    }));

    app::ButtonEvent ev;
    while (true)
    {
        if (xQueueReceive(q, &ev, portMAX_DELAY) == pdTRUE)
        {
            switch (ev)
            {
            case app::ButtonEvent::Press:
                g_led.next_mode();
                ESP_LOGI(TAG, "mode %u", static_cast<unsigned>(g_led.mode()));
                break;
            case app::ButtonEvent::LongPress:
                g_led.set_mode(app::LedMode::Off);
                break;
            case app::ButtonEvent::Release:
                break;
            }
        }
    }
}