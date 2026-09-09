#include "led.hpp"
#include "esp_check.h"

namespace app
{

  static const char *TAG = "led";

  esp_err_t Led::init(const Config &cfg)
  {
    ESP_RETURN_ON_FALSE(cfg.gpio != GPIO_NUM_NC, ESP_ERR_INVALID_ARG, TAG, "no gpio");
    ESP_RETURN_ON_FALSE(cfg.blink_period_ms >= 20, ESP_ERR_INVALID_ARG,
                        TAG, "period too short");

    cfg_ = cfg;

    const gpio_config_t io{
        .pin_bit_mask = 1ULL << cfg.gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&io), TAG, "gpio config");

    timer_ = xTimerCreateStatic("led_blink",
                                pdMS_TO_TICKS(cfg.blink_period_ms / 2),
                                pdTRUE,
                                this,
                                &timer_trampoline,
                                &timer_storage_);
    ESP_RETURN_ON_FALSE(timer_, ESP_ERR_NO_MEM, TAG, "timer create");

    set_mode(LedMode::Off);
    return ESP_OK;
  }

  Led::~Led()
  {
    if (timer_)
    {
      xTimerStop(timer_, 0);
      xTimerDelete(timer_, 0);
    }
  }

  void Led::write(bool on)
  {
    level_ = on;
    gpio_set_level(cfg_.gpio, static_cast<uint32_t>(on != cfg_.active_low));
  }

  void Led::set_mode(LedMode mode)
  {
    if (mode >= LedMode::Count)
      return;

    mode_ = mode;

    switch (mode)
    {
    case LedMode::Off:
      xTimerStop(timer_, 0);
      write(false);
      break;
    case LedMode::On:
      xTimerStop(timer_, 0);
      write(true);
      break;
    case LedMode::Blink:
      write(true);
      xTimerStart(timer_, 0);
      break;
    default:
      break;
    }
  }

  void Led::next_mode()
  {
    const auto n = static_cast<std::uint8_t>(mode_);
    set_mode(static_cast<LedMode>((n + 1) % static_cast<std::uint8_t>(LedMode::Count)));
  }

  void Led::timer_trampoline(TimerHandle_t t)
  {
    static_cast<Led *>(pvTimerGetTimerID(t))->on_blink_tick();
  }

  void Led::on_blink_tick()
  {
    write(!level_);
  }

}