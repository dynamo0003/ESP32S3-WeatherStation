#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "esp_log.h"
#include "private_define.h"

static const char * TAG = "HTTP";

static double s_temp, s_hum = 0;
static int s_id = 1;

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
                    cJSON *weather = cJSON_GetObjectItem(root, "weather");
                    cJSON *main = cJSON_GetObjectItem(root, "main");

                    if (cJSON_IsArray(weather)) {
                        cJSON *w0 = cJSON_GetArrayItem(weather, 0);
                        if (w0) {
                            cJSON *id = cJSON_GetObjectItem(w0, "id");
                            if (cJSON_IsNumber(id)) {
                                s_id = id->valueint;
                                ESP_LOGI(TAG, "Weather Condition ID: %d", s_id);
                            } else {
                                ESP_LOGW(TAG, "weather[0].id missing or not number");
                            }
                        } else {
                            ESP_LOGW(TAG, "weather array empty");
                        }
                    } else {
                        ESP_LOGW(TAG, "'weather' not an array");
                    }

                    if (main) {
                        cJSON *temp = cJSON_GetObjectItem(main, "temp");
                        cJSON *humidity = cJSON_GetObjectItem(main, "humidity");
                        if (cJSON_IsNumber(temp)) {
                            s_temp = temp->valuedouble;
                            ESP_LOGI(TAG, "Temperature: %.2f C", temp->valuedouble);
                        }
                        if (cJSON_IsNumber(humidity)) {
                            s_hum = humidity->valuedouble;
                            ESP_LOGI(TAG, "Humidity: %.2f %%", humidity->valuedouble);
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

void get_current_weather_data(void)
{
    char url[200];
    sprintf(url, "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric&lang=sl", OWM_LAT, OWM_LON, OWM_API_KEY);

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

void get_temp_humidity(double *ret_temperature, double *ret_humidity)
{
    *ret_temperature = s_temp;
    *ret_humidity = s_hum;
}

int get_weather_condition()
{
    int id = s_id;
    int ret;

    if(id == 800) { // Clear
        ret = 1;
    }
    else if(id == 801) { // Partly cloudy
        ret = 2;
    }
    else if(id >= 802 && id <= 804) { // Cloudy
        ret = 3;
    }
    else if((id >= 300 && id <= 321) || (id >= 500 && id <= 531)) { // Rain
        ret = 4;
    }
    else if(id >= 600 && id <= 622) { // Snow
        ret = 5;
    }
    else if(id >= 200 && id <= 232) { // Thunderstorm
        ret = 6;
    }
    else { // Unknown - dont change current condition
        ret = 0;
    }

    return ret;
}