#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";    

void app_main() {
    ESP_LOGI(TAG, "Starting application...");
    while (1) {
        ESP_LOGI(TAG, "Application is running...");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}
