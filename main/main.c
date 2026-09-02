/*
 * esp32-gui - entry point.
 *
 * All hardware specifics live in board_config.h; all bring-up in display.c.
 * This file only decides WHAT to draw once the panel is alive.
 */

#include "display.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"

/* Compiles against the SquareLine export as soon as one exists in
 * squareline_ui/, and falls back to a self-test screen until then. */
#if defined(__has_include)
#  if __has_include("ui.h")
#    include "ui.h"
#    define HAVE_SQUARELINE_UI 1
#  endif
#endif

static const char *TAG = "app";

#ifndef HAVE_SQUARELINE_UI
static void btn_event_cb(lv_event_t *e)
{
    lv_obj_t *label = lv_event_get_user_data(e);
    static int taps;
    lv_label_set_text_fmt(label, "touch ok: %d", ++taps);
}

/* Proves panel + LVGL + touch end to end. Delete once ui_init() takes over. */
static void placeholder_screen(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x11161c), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text_fmt(title, "LVGL %d.%d.%d", LVGL_VERSION_MAJOR,
                          LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe6edf3), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *status = lv_label_create(scr);
    lv_label_set_text(status, "touch ok: 0");
    lv_obj_set_style_text_color(status, lv_color_hex(0x7d8590), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -12);

    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_set_size(btn, 130, 46);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, status);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Tap me");
    lv_obj_center(btn_label);
}
#endif

void app_main(void)
{
    lv_display_t *disp = display_init();

    /* esp_lvgl_port runs LVGL on its own task; every LVGL call from
     * elsewhere must hold this lock. */
    if (lvgl_port_lock(0)) {
#ifdef HAVE_SQUARELINE_UI
        ui_init();
#else
        placeholder_screen(disp);
#endif
        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "ui ready");
}
