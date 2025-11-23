#include "screen1.h"
#include <stdio.h>
#include "defines.h"

// Module-level static variables for this screen
static lv_obj_t * screen_container;
static lv_obj_t * bg_image;
static lv_timer_t * update_timer;

LV_IMAGE_DECLARE(img_clear_day);
LV_IMAGE_DECLARE(img_cloudy_day);
LV_IMAGE_DECLARE(img_partlycloudy_day);
LV_IMAGE_DECLARE(img_rain_day);
LV_IMAGE_DECLARE(img_snowy_day);
LV_IMAGE_DECLARE(img_storm_day);

LV_IMAGE_DECLARE(icon_clock);
LV_IMAGE_DECLARE(icon_house);
LV_IMAGE_DECLARE(icon_outside);
LV_IMAGE_DECLARE(icon_sun);
LV_IMAGE_DECLARE(icon_cloudy);
LV_IMAGE_DECLARE(icon_partlycloudy);
LV_IMAGE_DECLARE(icon_rain);
LV_IMAGE_DECLARE(icon_snowy);
LV_IMAGE_DECLARE(icon_storm);

LV_FONT_DECLARE(inria_sans_14);
LV_FONT_DECLARE(inria_sans_20);
LV_FONT_DECLARE(inria_sans_24);
LV_FONT_DECLARE(inria_sans_32);
LV_FONT_DECLARE(inria_sans_52);
LV_FONT_DECLARE(inria_sans_96);

// Private function prototypes
static void update_timer_cb(lv_timer_t * timer);
static void draw_cb(lv_event_t* e);

lv_obj_t * screen1_create(lv_obj_t * parent) {
    // Create the main screen container
    // This is the object the screen_manager will track
    screen_container = lv_obj_create(parent);
    lv_obj_remove_style_all(screen_container);
    lv_obj_set_size(screen_container, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_clear_flag(screen_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_opa(screen_container, LV_OPA_COVER, LV_PART_MAIN);
    // lv_obj_add_event_cb(screen_container, draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    // bg image
    bg_image = lv_img_create(screen_container);
    lv_img_set_src(bg_image, &img_cloudy_day);
    lv_obj_set_size(bg_image, LCD_WIDTH, LCD_HEIGHT);

    // ---------------------------------------------------------------------------

    lv_obj_t* rect_time = lv_obj_create(screen_container);
    lv_obj_remove_style_all(rect_time);
    lv_obj_set_style_opa(rect_time, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_time, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_time, MAIN_RECT_STANDARD_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_time, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_time, 250, 270);
    lv_obj_set_pos(rect_time, 90, 180);

    lv_obj_t* img_time = lv_img_create(rect_time);
    lv_img_set_src(img_time, &icon_clock);
    lv_obj_set_size(img_time, 75, 75);
    lv_obj_set_pos(img_time, 168, 22);

    lv_obj_t* date_label = lv_label_create(rect_time);
    lv_obj_set_size(date_label, 235, 24);
    lv_obj_set_pos(date_label, 8, 120);
    lv_obj_set_style_text_font(date_label, &inria_sans_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(date_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(date_label, "Nedelja, 4. November");

    lv_obj_t* time_label = lv_label_create(rect_time);
    lv_obj_set_size(time_label, 235, 115);
    lv_obj_set_pos(time_label, 8, 136);
    lv_obj_set_style_text_font(time_label, &inria_sans_96, LV_PART_MAIN);
    lv_obj_set_style_text_color(time_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(time_label, "20:37");

    // ---------------------------------------------------------------------------

    lv_obj_t* rect_location = lv_obj_create(screen_container);
    lv_obj_remove_style_all(rect_location);
    lv_obj_set_style_opa(rect_location, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_location, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_location, MAIN_RECT_STANDARD_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_location, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_location, 400, 120);
    lv_obj_set_pos(rect_location, 370, 30);

    lv_obj_t* location_label = lv_label_create(rect_location);
    lv_obj_set_size(location_label, 205, 38);
    lv_obj_set_pos(location_label, 17, 22);
    lv_obj_set_style_text_font(location_label, &inria_sans_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(location_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(location_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(location_label, "Rimske Toplice");

    lv_obj_t* weather_label = lv_label_create(rect_location);
    lv_obj_set_size(weather_label, 207, 29);
    lv_obj_set_pos(weather_label, 17, 65);
    lv_obj_set_style_text_font(weather_label, &inria_sans_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(weather_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(weather_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_label_set_text(weather_label, "Jasno");

    lv_obj_t* img_weather = lv_img_create(rect_location);
    lv_img_set_src(img_weather, &icon_sun);
    lv_obj_set_size(img_weather, 75, 75);
    lv_obj_set_pos(img_weather, 303, 22);

    // ---------------------------------------------------------------------------

    lv_obj_t* rect_in = lv_obj_create(screen_container);
    lv_obj_remove_style_all(rect_in);
    lv_obj_set_style_opa(rect_in, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_in, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_in, MAIN_RECT_STANDARD_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_in, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_in, 400, 120);
    lv_obj_set_pos(rect_in, 370, 180);

    lv_obj_t* rect_in_in = lv_obj_create(rect_in);
    lv_obj_remove_style_all(rect_in_in);
    lv_obj_set_style_opa(rect_in_in, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_in_in, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_in_in, MAIN_RECT_HIGHER_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_in_in, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_in_in, 152, 34);
    lv_obj_set_pos(rect_in_in, 0, 0);

    lv_obj_t* inside_label = lv_label_create(rect_in);
    lv_obj_set_size(inside_label, 94, 17);
    lv_obj_set_pos(inside_label, 29, 8);
    lv_obj_set_style_text_font(inside_label, &inria_sans_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(inside_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(inside_label, "Notranje stanje");

    lv_obj_t* inside_temp_label = lv_label_create(rect_in);
    lv_obj_set_size(inside_temp_label, 165, 52);
    lv_obj_set_pos(inside_temp_label, 2, 44);
    lv_obj_set_style_text_font(inside_temp_label, &inria_sans_52, LV_PART_MAIN);
    lv_obj_set_style_text_color(inside_temp_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(inside_temp_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(inside_temp_label, "24.5°C");

    lv_obj_t* inside_hum_label = lv_label_create(rect_in);
    lv_obj_set_size(inside_hum_label, 121, 52);
    lv_obj_set_pos(inside_hum_label, 182, 44);
    lv_obj_set_style_text_font(inside_hum_label, &inria_sans_52, LV_PART_MAIN);
    lv_obj_set_style_text_color(inside_hum_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(inside_hum_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(inside_hum_label, "62%%");

    lv_obj_t* img_inside = lv_img_create(rect_in);
    lv_img_set_src(img_inside, &icon_house);
    lv_obj_set_size(img_inside, 75, 75);
    lv_obj_set_pos(img_inside, 303, 22);

    // ---------------------------------------------------------------------------

    lv_obj_t* rect_out = lv_obj_create(screen_container);
    lv_obj_remove_style_all(rect_out);
    lv_obj_set_style_opa(rect_out, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_out, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_out, MAIN_RECT_STANDARD_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_out, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_out, 400, 120);
    lv_obj_set_pos(rect_out, 370, 330);

    lv_obj_t* rect_in_out = lv_obj_create(rect_out);
    lv_obj_remove_style_all(rect_in_out);
    lv_obj_set_style_opa(rect_in_out, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect_in_out, MAIN_RECT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rect_in_out, MAIN_RECT_HIGHER_OPA, LV_PART_MAIN);
    lv_obj_set_style_radius(rect_in_out, 10, LV_PART_MAIN);
    lv_obj_set_size(rect_in_out, 152, 34);
    lv_obj_set_pos(rect_in_out, 0, 0);

    lv_obj_t* outside_label = lv_label_create(rect_out);
    lv_obj_set_size(outside_label, 94, 17);
    lv_obj_set_pos(outside_label, 29, 8);
    lv_obj_set_style_text_font(outside_label, &inria_sans_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(outside_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(outside_label, "Zunanje stanje");

    lv_obj_t* outside_temp_label = lv_label_create(rect_out);
    lv_obj_set_size(outside_temp_label, 165, 52);
    lv_obj_set_pos(outside_temp_label, 2, 44);
    lv_obj_set_style_text_font(outside_temp_label, &inria_sans_52, LV_PART_MAIN);
    lv_obj_set_style_text_color(outside_temp_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(outside_temp_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(outside_temp_label, "27.6°C");

    lv_obj_t* outside_hum_label = lv_label_create(rect_out);
    lv_obj_set_size(outside_hum_label, 121, 52);
    lv_obj_set_pos(outside_hum_label, 182, 44);
    lv_obj_set_style_text_font(outside_hum_label, &inria_sans_52, LV_PART_MAIN);
    lv_obj_set_style_text_color(outside_hum_label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(outside_hum_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(outside_hum_label, "83%%");

    lv_obj_t* img_outside = lv_img_create(rect_out);
    lv_img_set_src(img_outside, &icon_outside);
    lv_obj_set_size(img_outside, 75, 75);
    lv_obj_set_pos(img_outside, 303, 22);

    // ---------------------------------------------------------------------------

    // Create a timer for this screen
    // This demonstrates how to tie a timer to a screen's lifecycle
    update_timer = lv_timer_create(update_timer_cb, 1000, NULL);
    lv_timer_resume(update_timer);

    return screen_container;
}

void screen1_delete(void) {
    // --- 1. Delete all timers, tasks, etc. ---
    if (update_timer) {
        lv_timer_delete(update_timer);
        update_timer = NULL;
    }

    // --- 2. Delete all LVGL objects ---
    // Deleting the container will automatically delete its children (like the label)
    if (screen_container) {
        lv_obj_del(screen_container);
        screen_container = NULL;
    }
}

static void update_timer_cb(lv_timer_t * timer) {
    static int count = 0;
    count++;
    // lv_obj_invalidate(screen_container);
}

static void draw_cb(lv_event_t* e) {
    // lv_event_code_t code = lv_event_get_code(e);
    // lv_layer_t *layer = lv_event_get_layer(e);
    // if (code != LV_EVENT_DRAW_MAIN) return;

    // lv_draw_image_dsc_t img;
    // lv_draw_image_dsc_init(&img);
    // img.src = &icon_sun;
    // lv_area_t img_area = {200, 200, 275, 275};
    // lv_draw_image(layer, &img, &img_area);
}