#pragma once

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lists all available screens in the application.
 * Add your new screen types here.
 */
typedef enum {
    SCREEN_NONE,
    SCREEN_HOME,
    SCREEN_SETTINGS,
    // Add new screen enums here
} ScreenType;

/**
 * @brief Initializes the screen manager.
 */
void screen_manager_init(void);

/**
 * @brief Cleans up the current screen and displays a new one.
 * This is the primary function for screen navigation.
 *
 * @param type The enum of the screen to show.
 */
void screen_manager_show(ScreenType type);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_MANAGER_H