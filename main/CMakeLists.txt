idf_component_register(SRCS "main.c" "wifi.c" "http.c" "aht20.c"
                    INCLUDE_DIRS "."
                    REQUIRES "esp_wifi" "nvs_flash" "esp_http_client" "mbedtls" "json"
                    )
