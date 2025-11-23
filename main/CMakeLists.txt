file(GLOB SRC_FILES
    "*.c"
    "images/*.c"
    "fonts/*.c"
)

idf_component_register(SRCS ${SRC_FILES}
                    INCLUDE_DIRS "." "images"
                    REQUIRES "esp_wifi" "nvs_flash" "esp_http_client" "mbedtls" "json"
                    )
