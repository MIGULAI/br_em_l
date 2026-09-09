#pragma once

#include <cstdint>

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace app
{

  enum class LedMode : std::uint8_t
  {
    Off = 0,
    Blink,
    On,
    Count // sentinel: not a real mode
  };

  class Led
  {
  public:
    struct Config
    {
      gpio_num_t gpio{GPIO_NUM_NC};
      bool active_low{false};
      std::uint32_t blink_period_ms{500}; // full cycle: 250 on, 250 off
    };

    Led() = default;
    ~Led();

    Led(const Led &) = delete;
    Led &operator=(const Led &) = delete;
    Led(Led &&) = delete;
    Led &operator=(Led &&) = delete;

    [[nodiscard]] esp_err_t init(const Config &cfg);

    void set_mode(LedMode mode);
    void next_mode();
    [[nodiscard]] LedMode mode() const { return mode_; }

  private:
    static void timer_trampoline(TimerHandle_t t);
    void on_blink_tick();
    void write(bool on);

    Config cfg_{};
    LedMode mode_{LedMode::Off};
    bool level_{false};
    TimerHandle_t timer_{nullptr};
    StaticTimer_t timer_storage_{};
  };

}