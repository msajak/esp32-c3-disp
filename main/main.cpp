#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "shared_objs.h"
#include "web_server.h"
#include "wifi_mgmt.h"
#include "cdisplay.h"
#include "logger.h"

// ESP-IDF entry point
extern "C" {
    void app_main(void);
}

QueueHandle_t status_queue;

static constexpr const char* TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "ESP32C3 tinyLCD starting...");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS initialized");

    logger_init();
    ESP_LOGI(TAG, "Logger initialized");

    status_queue = xQueueCreate(1, sizeof(Status));
    if (!status_queue) {
        ESP_LOGE(TAG, "Failed to create status queue");
        return;
    }

    WifiMgmt wifi_mgmt;
    wifi_mgmt.start();

    WebServer server(status_queue, wifi_mgmt);
    server.start();

    ESP_LOGI(TAG, "ESP32C3 tinyLCD started! Web UI at http://%s/", wifi_mgmt.get_ip_str());

    CDisplay display;
    display.begin();


    int i = 0;
    uint8_t v= 0;
    while (1) {
        display.clear(v);
        // for (int y=0; y<40; y+= 8) {
        //     for (int x=( y & 8); x<60; x+= 16) {
        //         display.rect(x, y, 8, 8);
        //     }
        // }
        // char t[20]; sprintf(t, "test %d", i); display.text(t, 0, 12);

        display.rect(0, 0, 72, 1);
        display.rect(0, 39, 72, 1);
        display.rect(0, 0, 1, 40);
        display.rect(71, 0, 1, 40);

        char ip[20]; sprintf(ip, ".%u", static_cast<uint8_t>((wifi_mgmt.get_ip() >> 24) & 0xFF));
        display.text(ip, 3, 12);

        char t[20]; sprintf(t, "test %d", i);
        display.text(t, 3, 24);

        display.disp();
        ++i;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
