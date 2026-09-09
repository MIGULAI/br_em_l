#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace app
{

  enum class ButtonEvent : std::uint8_t
  {
    Press,
    Release,
    LongPress
  };

  class Button
  {
  public:
    struct Config
    {
      gpio_num_t gpio{GPIO_NUM_NC};
      bool active_low{true}; // typical: pull-up, switch to GND
      bool pull_up{true};
      std::uint32_t debounce_ms{30};
      std::uint32_t long_press_ms{1000};
      QueueHandle_t out_queue{nullptr}; // receives ButtonEvent
    };

    Button() = default;
    ~Button();

    Button(const Button &) = delete;
    Button &operator=(const Button &) = delete;
    Button(Button &&) = delete;
    Button &operator=(Button &&) = delete;

    [[nodiscard]] esp_err_t init(const Config &cfg);
    [[nodiscard]] bool pressed() const;

  private:
    static void IRAM_ATTR isr_trampoline(void *arg);
    void IRAM_ATTR on_edge();

    static void timer_trampoline(TimerHandle_t t);
    void on_debounce_expired();

    Config cfg_{};
    TimerHandle_t debounce_timer_{nullptr};
    StaticTimer_t timer_storage_{};
    volatile bool last_stable_{false};
    std::int64_t press_started_us_{0};
  };
}