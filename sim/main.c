/*
 * Desktop LVGL simulator for the esp32-gui project.
 *
 * Compiles the SAME squareline_ui/ sources the ESP32 firmware compiles, against
 * the SAME LVGL version (9.5.0), rendered into an SDL2 window instead of a panel.
 *
 * If squareline_ui/ contains a SquareLine export (ui.h present), this runs it.
 * Otherwise it draws a placeholder so the harness is testable before the export.
 */

#include "lvgl.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

#if defined(__has_include)
#  if __has_include("ui.h")
#    include "ui.h"
#    define HAVE_SQUARELINE_UI 1
#  endif
#endif

#ifndef HAVE_SQUARELINE_UI
static void btn_event_cb(lv_event_t *e)
{
    lv_obj_t *label = lv_event_get_user_data(e);
    static int taps;
    lv_label_set_text_fmt(label, "input ok: %d", ++taps);
}

static void placeholder_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x11161c), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text_fmt(title, "LVGL %d.%d.%d", LVGL_VERSION_MAJOR,
                          LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe6edf3), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "squareline_ui/ is empty");
    lv_obj_set_style_text_color(sub, lv_color_hex(0x7d8590), 0);
    lv_obj_align_to(sub, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    lv_obj_t *status = lv_label_create(scr);
    lv_label_set_text(status, "input ok: 0");
    lv_obj_set_style_text_color(status, lv_color_hex(0x7d8590), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -18);

    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_set_size(btn, 140, 48);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, status);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Tap me");
    lv_obj_center(btn_label);
}
#endif

int main(void)
{
    lv_init();
    lv_tick_set_cb(SDL_GetTicks);   /* LV_USE_OS is NONE; SDL provides the ms tick */

    lv_display_t *disp = lv_sdl_window_create(SIM_HOR_RES, SIM_VER_RES);
    lv_sdl_window_set_zoom(disp, SIM_ZOOM);

    lv_sdl_mouse_create();          /* stands in for the touch panel */
    lv_sdl_mousewheel_create();
    lv_sdl_keyboard_create();

#ifdef HAVE_SQUARELINE_UI
    ui_init();
#else
    placeholder_screen();
#endif

    while (true) {
        lv_timer_handler();
        SDL_Delay(5);
    }
    return 0;
}
