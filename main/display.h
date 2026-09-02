/*
 * Panel + touch + LVGL bring-up, driven entirely by board_config.h.
 * Nothing here should need editing for a new board.
 */
#pragma once

#include "lvgl.h"

/**
 * @brief Bring up the backlight, LCD panel, optional touch, and esp_lvgl_port.
 *
 * Safe to call once from app_main(). Aborts via ESP_ERROR_CHECK on any
 * hardware failure, so a successful return means the panel is live.
 *
 * @return the LVGL display handle, for lv_display_get_screen_active() etc.
 */
lv_display_t *display_init(void);
