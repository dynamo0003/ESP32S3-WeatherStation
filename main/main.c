#include <stdio.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "wifi.h"
#include "http.h"
#include "private_define.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char * TAG = "main";

void app_main(void)
{
    ESP_ERROR_CHECK(weather_wifi_init());

    esp_err_t ret = weather_wifi_connect(WIFI_SSID_R, WIFI_PASSWORD_R);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    }

    wifi_ap_record_t ap_info;
    ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_ERR_WIFI_CONN) {
        ESP_LOGE(TAG, "Wi-Fi station interface not initialized");
    }
    else if (ret == ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGE(TAG, "Wi-Fi station is not connected");
    } else {
        ESP_LOGI(TAG, "Wifi Connected, calling openweathermap api");
        get_current_weather_data();
    }

    while(1) {
        vTaskDelay(1000);
    }
}
