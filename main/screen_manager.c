#include "screen_manager.h"

#include "screen1.h"
// #include "screen_settings.h"
#include "defines.h"
#include "esp_lvgl_port.h"

// Module-level static variables to track the current screen
static lv_obj_t * current_screen = NULL;
static ScreenType current_screen_type = SCREEN_NONE;

lv_obj_t* parent_obj;
lv_obj_t* menu_stripe_obj;

/**
 * @brief Initializes the screen manager.
 */
void screen_manager_init(void) {
    current_screen = NULL;
    current_screen_type = SCREEN_NONE;
    
    lvgl_port_lock(0);

    menu_stripe_obj = lv_obj_create(lv_layer_top());
    lv_obj_set_size(menu_stripe_obj, 60, 480);
    lv_obj_set_style_bg_color(menu_stripe_obj, MENU_STRIPE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_opa(menu_stripe_obj, MENU_STRIPE_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_width(menu_stripe_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(menu_stripe_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(menu_stripe_obj, 0, LV_PART_MAIN);

    // TODO: add menu buttons and logic
    lv_obj_t* menu_button_home = lv_button_create(menu_stripe_obj);
    lv_obj_remove_style_all(menu_button_home);
    lv_obj_set_size(menu_button_home, 45, 45);
    lv_obj_set_pos(menu_button_home, 8, 12);
    lv_obj_set_style_bg_color(menu_button_home, MENU_BUTTON_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(menu_button_home, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(menu_button_home, 5, LV_PART_MAIN);

    lv_obj_t* home_label = lv_label_create(menu_button_home);
    lv_obj_set_size(home_label, 34, 34);
    lv_obj_align(home_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(home_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(home_label, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(home_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_text(home_label, LV_SYMBOL_HOME);
    
    lvgl_port_unlock();
}

/**
 * @brief Cleans up the current screen and displays a new one.
 */
void screen_manager_show(ScreenType type) {
    // Do nothing if the requested screen is already active
    if (type == current_screen_type) {
        return;
    }

    // Lock the display while we modify the UI
    lvgl_port_lock(0);

    // --- 1. Clean up the old screen ---
    if (current_screen != NULL) {
        // Call the specific delete function for the current screen
        // This function is responsible for deleting all objects, timers, tasks, etc.
        switch (current_screen_type) {
            case SCREEN_HOME:
                screen1_delete();
                break;
            case SCREEN_SETTINGS:
                // screen_settings_delete();
                break;
            default:
                // Fallback for screens without a specific delete function
                lv_obj_del(current_screen);
                break;
        }
    }

    // --- 2. Create the new screen ---
    // Call the specific create function for the new screen
    switch (type) {
        case SCREEN_HOME:
            current_screen = screen1_create(lv_screen_active());
            break;
        case SCREEN_SETTINGS:
            // current_screen = screen_settings_create(lv_screen_active());
            break;
        default:
            current_screen = NULL; // Or create a "Screen Not Found" error page
            break;
    }

    // Update the tracker
    current_screen_type = type;

    // Unlock the display
    lvgl_port_unlock();
}