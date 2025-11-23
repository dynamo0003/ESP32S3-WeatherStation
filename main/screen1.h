#pragma once

#ifndef SCREEN_1_H
#define SCREEN_1_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create basic Screen.
 *
 * @param parent The parent object (usually lv_screen_active()).
 * @return A pointer to the created screen's base container.
 */
lv_obj_t * screen1_create(lv_obj_t * parent);

/**
 * @brief Delete the Screen.
 * This function must clean up all objects, timers, tasks, etc.
 * created by screen_home_create.
 */
void screen1_delete(void);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_1_H