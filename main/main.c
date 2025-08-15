#include <stdio.h>

#include "esp_log.h"
#include "esp_wifi.h"

#include "wifi.h"

#include "freertos/task.h"

#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char * TAG = "main";

#define WIFI_SSID "Bajta Gorenc"
#define WIFI_PASSWORD "leonziga"

#define OWM_API_KEY "909bd847a28459008a557c57556ce5f7"
#define OWM_LAT "46.0569" // Latitude for Ljubljana
#define OWM_LON "14.5058" // Longitude for Ljubljana

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // If user_data buffer is configured, copy the response into it
            if (evt->user_data) {
                memcpy(evt->user_data + output_len, evt->data, evt->data_len);
            } else {
                // Allocate buffer for response data
                if (output_buffer == NULL) {
                    output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL) {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
            }
            output_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Now parse it.
                cJSON *root = cJSON_Parse(output_buffer);
                if (root) {
                    cJSON *main = cJSON_GetObjectItem(root, "main");
                    if (main) {
                        cJSON *temp = cJSON_GetObjectItem(main, "temp");
                        if (cJSON_IsNumber(temp)) {
                            ESP_LOGI(TAG, "Temperature: %.2f K", temp->valuedouble);
                        }
                    }
                    cJSON_Delete(root);
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON");
                }
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            // Free the buffer if we are disconnected before finishing
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

void get_weather_data(void)
{
    char url[200];
    sprintf(url, "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s", OWM_LAT, OWM_LON, OWM_API_KEY);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .user_agent = "esp32-weather-station/1.0",
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                (int)esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting tutorial...");
    ESP_ERROR_CHECK(weather_wifi_init());

    esp_err_t ret = weather_wifi_connect(WIFI_SSID, WIFI_PASSWORD);
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
        ESP_LOGI(TAG, "--- Access Point Information ---");
        ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
        ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
        ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
        ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);

        ESP_LOGI(TAG, "Calling openweathermap");
        get_weather_data();
    }

    while(1) {
        vTaskDelay(1000);
    }

    //ESP_ERROR_CHECK(weather_wifi_disconnect());

    //ESP_ERROR_CHECK(weather_wifi_deinit());

    ESP_LOGI(TAG, "End of tutorial...");
}
