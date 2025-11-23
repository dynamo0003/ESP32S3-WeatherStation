#include <stdio.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_idf_version.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_touch_gt911.h"
#include "wifi.h"
#include "http.h"
#include "private_define.h"
#include "aht20.h"
#include "screen_manager.h"
#include <stdbool.h>

#define MAX_HTTP_OUTPUT_BUFFER 2048

#define ENABLE_LV_LOG   (0)

static const char * TAG = "main";

/* LCD size */
#define EXAMPLE_LCD_H_RES   (800)
#define EXAMPLE_LCD_V_RES   (480)

/* LCD settings */
#define EXAMPLE_LCD_LVGL_FULL_REFRESH           (0)
#define EXAMPLE_LCD_LVGL_DIRECT_MODE            (1)
#define EXAMPLE_LCD_LVGL_AVOID_TEAR             (1)
#define EXAMPLE_LCD_RGB_BOUNCE_BUFFER_MODE      (1)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE            (1)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT            (100)
#define EXAMPLE_LCD_RGB_BUFFER_NUMS             (2)
#define EXAMPLE_LCD_RGB_BOUNCE_BUFFER_HEIGHT    (10)

/* LCD pins */
#define EXAMPLE_LCD_GPIO_VSYNC     (GPIO_NUM_41)
#define EXAMPLE_LCD_GPIO_HSYNC     (GPIO_NUM_39)
#define EXAMPLE_LCD_GPIO_DE        (GPIO_NUM_40)
#define EXAMPLE_LCD_GPIO_PCLK      (GPIO_NUM_42)
#define EXAMPLE_LCD_GPIO_DISP      (GPIO_NUM_NC)

#define EXAMPLE_LCD_GPIO_DATA0     (GPIO_NUM_45)
#define EXAMPLE_LCD_GPIO_DATA1     (GPIO_NUM_48)
#define EXAMPLE_LCD_GPIO_DATA2     (GPIO_NUM_47)
#define EXAMPLE_LCD_GPIO_DATA3     (GPIO_NUM_21)
#define EXAMPLE_LCD_GPIO_DATA4     (GPIO_NUM_14)
#define EXAMPLE_LCD_GPIO_DATA5     (GPIO_NUM_5)
#define EXAMPLE_LCD_GPIO_DATA6     (GPIO_NUM_6)
#define EXAMPLE_LCD_GPIO_DATA7     (GPIO_NUM_7)
#define EXAMPLE_LCD_GPIO_DATA8     (GPIO_NUM_15)
#define EXAMPLE_LCD_GPIO_DATA9     (GPIO_NUM_16)
#define EXAMPLE_LCD_GPIO_DATA10    (GPIO_NUM_4)
#define EXAMPLE_LCD_GPIO_DATA11    (GPIO_NUM_8)
#define EXAMPLE_LCD_GPIO_DATA12    (GPIO_NUM_3)
#define EXAMPLE_LCD_GPIO_DATA13    (GPIO_NUM_46)
#define EXAMPLE_LCD_GPIO_DATA14    (GPIO_NUM_9)
#define EXAMPLE_LCD_GPIO_DATA15    (GPIO_NUM_1)

/* Touch settings */
#define EXAMPLE_TOUCH_I2C_NUM       (0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ    (400000)

/* LCD touch pins */
#define EXAMPLE_TOUCH_I2C_SCL       (GPIO_NUM_20)
#define EXAMPLE_TOUCH_I2C_SDA       (GPIO_NUM_19)

#define EXAMPLE_LCD_PANEL_35HZ_RGB_TIMING()      \
    {                                            \
        .pclk_hz = 25 * 1000 * 1000,             \
        .h_res = EXAMPLE_LCD_H_RES,              \
        .v_res = EXAMPLE_LCD_V_RES,              \
        .hsync_pulse_width = 4,/*max:8*/         \
        .hsync_back_porch = 8,/*max:48*/         \
        .hsync_front_porch = 8,/*max:48*/        \
        .vsync_pulse_width = 4,/*max:8*/         \
        .vsync_back_porch = 8,/*max:12*/         \
        .vsync_front_porch = 8,/*max:12*/        \
        .flags.pclk_active_neg = true,           \
    }

/* LCD IO and panel */
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD initialization */
    ESP_LOGI(TAG, "Initialize RGB panel");
    esp_lcd_rgb_panel_config_t panel_conf = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 3, 0)
        .psram_trans_align = 64,
#else
        .dma_burst_size = 64,
#endif
        .data_width = 16,
        .bits_per_pixel = 16,
        .de_gpio_num = EXAMPLE_LCD_GPIO_DE,
        .pclk_gpio_num = EXAMPLE_LCD_GPIO_PCLK,
        .vsync_gpio_num = EXAMPLE_LCD_GPIO_VSYNC,
        .hsync_gpio_num = EXAMPLE_LCD_GPIO_HSYNC,
        .disp_gpio_num = EXAMPLE_LCD_GPIO_DISP,
        .data_gpio_nums = {
            EXAMPLE_LCD_GPIO_DATA11,
            EXAMPLE_LCD_GPIO_DATA12,
            EXAMPLE_LCD_GPIO_DATA13,
            EXAMPLE_LCD_GPIO_DATA14,
            EXAMPLE_LCD_GPIO_DATA15,
            
            EXAMPLE_LCD_GPIO_DATA5,
            EXAMPLE_LCD_GPIO_DATA6,
            EXAMPLE_LCD_GPIO_DATA7,
            EXAMPLE_LCD_GPIO_DATA8,
            EXAMPLE_LCD_GPIO_DATA9,
            EXAMPLE_LCD_GPIO_DATA10,
            
            EXAMPLE_LCD_GPIO_DATA0,
            EXAMPLE_LCD_GPIO_DATA1,
            EXAMPLE_LCD_GPIO_DATA2,
            EXAMPLE_LCD_GPIO_DATA3,
            EXAMPLE_LCD_GPIO_DATA4,
        },
        .timings = EXAMPLE_LCD_PANEL_35HZ_RGB_TIMING(),
        .flags.fb_in_psram = 1,
        .num_fbs = EXAMPLE_LCD_RGB_BUFFER_NUMS,
#if EXAMPLE_LCD_RGB_BOUNCE_BUFFER_MODE
        .bounce_buffer_size_px = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_RGB_BOUNCE_BUFFER_HEIGHT,
#endif
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_rgb_panel(&panel_conf, &lcd_panel), err, TAG, "RGB init failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(lcd_panel), err, TAG, "LCD init failed");

    return ret;

err:
    if (lcd_panel) {
        esp_lcd_panel_del(lcd_panel);
    }
    return ret;
}

static esp_err_t app_touch_init(void)
{
    /* Initilize I2C */
    i2c_master_bus_handle_t i2c_handle = NULL;
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = EXAMPLE_TOUCH_I2C_NUM,
        .sda_io_num = EXAMPLE_TOUCH_I2C_SDA,
        .scl_io_num = EXAMPLE_TOUCH_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = 1,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_config, &i2c_handle), TAG, "");

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_38,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = 100000; // TODO: try 200k
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(i2c_handle, &tp_io_config, &tp_io_handle), TAG, "");
    return esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle);
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,         /* LVGL task priority */
        .task_stack = 6144,         /* LVGL task stack size */
        .task_affinity = -1,        /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500,   /* Maximum sleep in LVGL task */
        .timer_period_ms = 5        /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    uint32_t buff_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT;
#if EXAMPLE_LCD_LVGL_FULL_REFRESH || EXAMPLE_LCD_LVGL_DIRECT_MODE
    buff_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES;
#endif

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = lcd_panel,
        .buffer_size = buff_size,
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
#if EXAMPLE_LCD_LVGL_FULL_REFRESH
            .full_refresh = true,
#elif EXAMPLE_LCD_LVGL_DIRECT_MODE
            .direct_mode = true,
#endif
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false,
#endif
        }
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
#if EXAMPLE_LCD_RGB_BOUNCE_BUFFER_MODE
            .bb_mode = true,
#else
            .bb_mode = false,
#endif
#if EXAMPLE_LCD_LVGL_AVOID_TEAR
            .avoid_tearing = true,
#else
            .avoid_tearing = false,
#endif
        }
    };
    lvgl_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    // lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

static void _app_button_cb(lv_event_t *e)
{
    lv_disp_rotation_t rotation = lv_disp_get_rotation(lvgl_disp);
    rotation++;
    if (rotation > LV_DISPLAY_ROTATION_270) {
        rotation = LV_DISPLAY_ROTATION_0;
    }

    /* LCD HW rotation */
    lv_disp_set_rotation(lvgl_disp, rotation);
}

static void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* Your LVGL objects code here .... */

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, EXAMPLE_LCD_H_RES);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label, LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"\n "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING);
}

#if ENABLE_LV_LOG
void my_log_cb(lv_log_level_t level, const char * buf)
{
  ESP_LOGI("LVGL", "%s", buf);
}
#endif

void app_main(void)
{
    ESP_ERROR_CHECK(weather_wifi_init());

    esp_err_t ret = weather_wifi_connect(WIFI_SSID, WIFI_PASSWORD);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi network");
    }

    // Backlight
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_2,
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_2, 1);
    
    ESP_ERROR_CHECK(app_lcd_init());
    // ESP_ERROR_CHECK(app_touch_init());
    ESP_ERROR_CHECK(app_lvgl_init());

    // Initialize the screen manager
    screen_manager_init();
    screen_manager_show(SCREEN_HOME);

    aht20_dev_handle_t aht20 = i2c_sensor_ath20_init();
    uint32_t temperature_raw;
    float temperature;
    uint32_t humidity_raw;
    float humidity;

    wifi_ap_record_t ap_info;
    double outside_temp, outside_humidity;

    uint32_t one_hour_counter = 3600, one_day_counter = 0;
    bool warning_wifi = false, warning_i2c = false;

    uint64_t current_time_ms = 0, current_date; // current time in ms since midnight

    #if ENABLE_LV_LOG
    lvgl_port_lock(0);
    lv_log_register_print_cb(my_log_cb);
    lvgl_port_unlock();
    #endif

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        one_hour_counter++; // value in seconds
        one_day_counter++;  // -||-

        ret = aht20_read_temperature_humidity(aht20, &temperature_raw, &temperature, &humidity_raw, &humidity);
        if(ret != ESP_OK) warning_i2c = true;
        else warning_i2c = false;
        ESP_LOGI("AHT20 READ", "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);

        if(one_hour_counter >= 3600) {
            // Call openweathermap api for current weather data

            ret = esp_wifi_sta_get_ap_info(&ap_info);
            if (ret == ESP_OK) {
                one_hour_counter = 0;
                get_current_weather_data();

                // Outside temp and humidity
                get_temp_humidity(&outside_temp, &outside_humidity);
                ESP_LOGI(TAG, "Outside Temperature: %.2f C, Humidity: %.2f %%", outside_temp, outside_humidity);

                // TODO: get current weather (sunny/rain/snow etc.)
            }
            else {
                ESP_LOGE(TAG, "Failed to get AP info: %s", esp_err_to_name(ret));
                warning_wifi = true; // Set warning flag to display on wifi error
                one_hour_counter = 3540; // Retry in 1 minute
            }
        }

        if(one_day_counter >= 86400) {
            one_day_counter = 0;
            // TODO: All of these
            // Call timezoneDB api for current time and date

            // current_time_ms = ...
            // current_date = ...

            // current_time_ms gets refreshed to acurate time every day
            // in the meantime, update the current_time_ms with one_day_counter

            // Parse day/night
        }
    }
}
