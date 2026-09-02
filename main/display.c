#include "display.h"
#include "board_config.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#if BOARD_LCD_BUS == BUS_SPI
#  include "driver/spi_master.h"
#  include "esp_lcd_panel_io.h"
#  include "esp_lcd_panel_vendor.h"
#  if BOARD_LCD_PANEL == PANEL_ILI9341
#    include "esp_lcd_ili9341.h"
#  elif BOARD_LCD_PANEL == PANEL_GC9A01
#    include "esp_lcd_gc9a01.h"
#  elif BOARD_LCD_PANEL == PANEL_ST7796
#    include "esp_lcd_st7796.h"
#  endif
#elif BOARD_LCD_BUS == BUS_RGB
#  include "esp_lcd_panel_rgb.h"
#endif

#if BOARD_TOUCH != TOUCH_NONE
#  include "driver/i2c_master.h"
#  include "esp_lcd_io_i2c.h"
#  include "esp_lcd_touch.h"
#  if BOARD_TOUCH == TOUCH_GT911
#    include "esp_lcd_touch_gt911.h"
#  elif BOARD_TOUCH == TOUCH_FT5X06
#    include "esp_lcd_touch_ft5x06.h"
#  elif BOARD_TOUCH == TOUCH_CST816S
#    include "esp_lcd_touch_cst816s.h"
#  endif
#endif

static const char *TAG = "display";

/* ------------------------------------------------------------------ */
/* Backlight                                                           */
/* ------------------------------------------------------------------ */
static void backlight_init(void)
{
#if BOARD_PIN_LCD_BL >= 0
    const gpio_config_t cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << BOARD_PIN_LCD_BL,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    /* Off during panel init so the user never sees uninitialised noise. */
    gpio_set_level(BOARD_PIN_LCD_BL, !BOARD_LCD_BL_ON_LEVEL);
#endif
}

static void backlight_on(void)
{
#if BOARD_PIN_LCD_BL >= 0
    gpio_set_level(BOARD_PIN_LCD_BL, BOARD_LCD_BL_ON_LEVEL);
#endif
}

/* ------------------------------------------------------------------ */
/* Touch (all supported controllers are I2C)                           */
/* ------------------------------------------------------------------ */
#if BOARD_TOUCH != TOUCH_NONE
static esp_lcd_touch_handle_t touch_init(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = BOARD_TOUCH_I2C_PORT,
        .sda_io_num = BOARD_PIN_TOUCH_SDA,
        .scl_io_num = BOARD_PIN_TOUCH_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

#if BOARD_TOUCH == TOUCH_GT911
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
#elif BOARD_TOUCH == TOUCH_FT5X06
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
#elif BOARD_TOUCH == TOUCH_CST816S
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
#endif
    io_cfg.scl_speed_hz = BOARD_TOUCH_I2C_HZ;

    esp_lcd_panel_io_handle_t io = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(bus, &io_cfg, &io));

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BOARD_LCD_H_RES,
        .y_max = BOARD_LCD_V_RES,
        .rst_gpio_num = BOARD_PIN_TOUCH_RST,
        .int_gpio_num = BOARD_PIN_TOUCH_INT,
        .levels = { .reset = 0, .interrupt = 0 },
        .flags = {
            .swap_xy  = BOARD_LCD_SWAP_XY,
            .mirror_x = BOARD_LCD_MIRROR_X,
            .mirror_y = BOARD_LCD_MIRROR_Y,
        },
    };

    esp_lcd_touch_handle_t tp = NULL;
#if BOARD_TOUCH == TOUCH_GT911
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io, &tp_cfg, &tp));
#elif BOARD_TOUCH == TOUCH_FT5X06
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(io, &tp_cfg, &tp));
#elif BOARD_TOUCH == TOUCH_CST816S
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(io, &tp_cfg, &tp));
#endif
    ESP_LOGI(TAG, "touch controller ready");
    return tp;
}
#endif /* BOARD_TOUCH != TOUCH_NONE */

/* ------------------------------------------------------------------ */
/* Public entry point                                                  */
/* ------------------------------------------------------------------ */
lv_display_t *display_init(void)
{
    backlight_init();

    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_io_handle_t panel_io = NULL;

#if BOARD_LCD_BUS == BUS_SPI
    const spi_bus_config_t bus_cfg = {
        .sclk_io_num = BOARD_PIN_LCD_SCLK,
        .mosi_io_num = BOARD_PIN_LCD_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BOARD_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(BOARD_LCD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    const esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = BOARD_PIN_LCD_DC,
        .cs_gpio_num = BOARD_PIN_LCD_CS,
        .pclk_hz = BOARD_LCD_PCLK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)BOARD_LCD_SPI_HOST, &io_cfg, &panel_io));

    const esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = BOARD_PIN_LCD_RST,
        .rgb_ele_order = BOARD_LCD_BGR ? LCD_RGB_ELEMENT_ORDER_BGR
                                       : LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };

#  if BOARD_LCD_PANEL == PANEL_ST7789
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_cfg, &panel));
#  elif BOARD_LCD_PANEL == PANEL_ILI9341
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_cfg, &panel));
#  elif BOARD_LCD_PANEL == PANEL_GC9A01
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(panel_io, &panel_cfg, &panel));
#  elif BOARD_LCD_PANEL == PANEL_ST7796
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(panel_io, &panel_cfg, &panel));
#  endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
#  if BOARD_LCD_INVERT
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));
#  endif
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, BOARD_LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, BOARD_LCD_MIRROR_X, BOARD_LCD_MIRROR_Y));
#  if BOARD_LCD_GAP_X || BOARD_LCD_GAP_Y
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel, BOARD_LCD_GAP_X, BOARD_LCD_GAP_Y));
#  endif
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

#elif BOARD_LCD_BUS == BUS_RGB
    const esp_lcd_rgb_panel_config_t rgb_cfg = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_width = 16,
        .bits_per_pixel = 16,
        .num_fbs = 2,
        .dma_burst_size = 64,
        .hsync_gpio_num = BOARD_PIN_RGB_HSYNC,
        .vsync_gpio_num = BOARD_PIN_RGB_VSYNC,
        .de_gpio_num = BOARD_PIN_RGB_DE,
        .pclk_gpio_num = BOARD_PIN_RGB_PCLK,
        .disp_gpio_num = BOARD_PIN_RGB_DISP,
        .data_gpio_nums = BOARD_RGB_DATA_GPIOS,
        .timings = {
            .pclk_hz = BOARD_RGB_PCLK_HZ,
            .h_res = BOARD_LCD_H_RES,
            .v_res = BOARD_LCD_V_RES,
            .hsync_back_porch = BOARD_RGB_HSYNC_BACK_PORCH,
            .hsync_front_porch = BOARD_RGB_HSYNC_FRONT_PORCH,
            .hsync_pulse_width = BOARD_RGB_HSYNC_PULSE_WIDTH,
            .vsync_back_porch = BOARD_RGB_VSYNC_BACK_PORCH,
            .vsync_front_porch = BOARD_RGB_VSYNC_FRONT_PORCH,
            .vsync_pulse_width = BOARD_RGB_VSYNC_PULSE_WIDTH,
            .flags.pclk_active_neg = BOARD_RGB_PCLK_ACTIVE_NEG,
        },
        .flags.fb_in_psram = true,
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
#endif

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .hres = BOARD_LCD_H_RES,
        .vres = BOARD_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy  = BOARD_LCD_SWAP_XY,
            .mirror_x = BOARD_LCD_MIRROR_X,
            .mirror_y = BOARD_LCD_MIRROR_Y,
        },
    };

#if BOARD_LCD_BUS == BUS_SPI
    /* Partial buffers in DMA-capable internal RAM; SPI needs the byte swap. */
    disp_cfg.buffer_size = BOARD_LCD_H_RES * 40;
    disp_cfg.double_buffer = true;
    disp_cfg.flags.buff_dma = true;
    disp_cfg.flags.swap_bytes = true;
    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
#else
    /* RGB panels own their framebuffers in PSRAM; no swap, no DMA buffer. */
    disp_cfg.buffer_size = BOARD_LCD_H_RES * BOARD_LCD_V_RES;
    disp_cfg.double_buffer = true;
    disp_cfg.flags.buff_spiram = true;
    const lvgl_port_display_rgb_cfg_t rgb_lvgl_cfg = {
        .flags = { .bb_mode = false, .avoid_tearing = true },
    };
    lv_display_t *disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_lvgl_cfg);
#endif
    assert(disp);

#if BOARD_TOUCH != TOUCH_NONE
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = touch_init(),
    };
    assert(lvgl_port_add_touch(&touch_cfg));
#endif

    backlight_on();
    ESP_LOGI(TAG, "display up: %dx%d", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return disp;
}
