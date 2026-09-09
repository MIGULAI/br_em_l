#include "button.hpp"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/timers.h"

namespace app
{
  static const char *TAG = "button";

  esp_err_t Button::init(const Config &cfg)
  {
    ESP_RETURN_ON_FALSE(cfg.gpio != GPIO_NUM_NC, ESP_ERR_INVALID_ARG, TAG, "no gpio");
    ESP_RETURN_ON_FALSE(cfg.out_queue != nullptr, ESP_ERR_INVALID_ARG, TAG, "no queue");

    cfg_ = cfg;

    const gpio_config_t io{
        .pin_bit_mask = 1ULL << cfg.gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = cfg.pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = cfg.pull_up ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&io), TAG, "gpio config");

    debounce_timer_ = xTimerCreateStatic("btn_db", pdMS_TO_TICKS(cfg.debounce_ms),
                                         pdFALSE, this,
                                         &timer_trampoline, &timer_storage_);
    ESP_RETURN_ON_FALSE(debounce_timer_, ESP_ERR_NO_MEM, TAG, "timer");

    last_stable_ = pressed();

    // Idempotent across multiple buttons; ALREADY installed is not an error.
    const esp_err_t err = gpio_install_isr_service(0);
    ESP_RETURN_ON_FALSE(err == ESP_OK || err == ESP_ERR_INVALID_STATE,
                        err, TAG, "isr service");

    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(cfg.gpio, &isr_trampoline, this),
                        TAG, "isr add");
    return ESP_OK;
  }

  Button::~Button()
  {
    if (cfg_.gpio != GPIO_NUM_NC)
      gpio_isr_handler_remove(cfg_.gpio);
    if (debounce_timer_)
      xTimerDelete(debounce_timer_, 0);
  }

  void Button::isr_trampoline(void *arg)
  {
    static_cast<Button *>(arg)->on_edge();
  }

  void Button::on_edge()
  {
    // ISR does one thing: restart the debounce timer. Nothing else.
    BaseType_t hp_woken = pdFALSE;
    xTimerResetFromISR(debounce_timer_, &hp_woken);
    if (hp_woken)
      portYIELD_FROM_ISR();
  }

  void Button::timer_trampoline(TimerHandle_t t)
  {
    static_cast<Button *>(pvTimerGetTimerID(t))->on_debounce_expired();
  }

  void Button::on_debounce_expired()
  {
    // Runs in the timer service task, not an ISR. Safe to do real work.
    const bool now = pressed();
    if (now == last_stable_)
      return; // spurious edge, level unchanged

    last_stable_ = now;

    ButtonEvent ev;
    if (now)
    {
      press_started_us_ = esp_timer_get_time();
      ev = ButtonEvent::Press;
    }
    else
    {
      const std::int64_t held_us = esp_timer_get_time() - press_started_us_;
      ev = (held_us > static_cast<std::int64_t>(cfg_.long_press_ms) * 1000)
               ? ButtonEvent::LongPress
               : ButtonEvent::Release;
    }

    xQueueSend(cfg_.out_queue, &ev, 0); // never block in a service task
  }

  bool Button::pressed() const
  {
    return gpio_get_level(cfg_.gpio) != static_cast<int>(cfg_.active_low);
  }
}
