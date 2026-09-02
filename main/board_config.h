/*
 * ============================================================================
 *  board_config.h  --  THE ONLY FILE YOU SHOULD NEED TO EDIT FOR A NEW BOARD
 * ============================================================================
 *
 *  Two steps:
 *    1. Set BOARD_PROFILE to whichever panel you have.
 *    2. Fill in the GPIO numbers in that profile's PINS block from your
 *       board's schematic or product page.
 *
 *  The pin numbers shipped here are PLACEHOLDERS. They are not your board's
 *  pinout and will not work until you replace them. Everything else in a
 *  profile (driver choice, resolution, colour order, timings) is already
 *  correct for that panel family.
 * ============================================================================
 */
#pragma once

/* --- bus kinds ------------------------------------------------------------ */
#define BUS_SPI          1
#define BUS_RGB          2

/* --- panel controllers ---------------------------------------------------- */
#define PANEL_ST7789     1
#define PANEL_ILI9341    2
#define PANEL_GC9A01     3
#define PANEL_ST7796     4
#define PANEL_RGB        5

/* --- touch controllers ---------------------------------------------------- */
#define TOUCH_NONE       0
#define TOUCH_GT911      1
#define TOUCH_FT5X06     2
#define TOUCH_CST816S    3

/* --- board profiles ------------------------------------------------------- */
#define BOARD_ST7789_SPI     1   /* 240x320 SPI, very common bare panel      */
#define BOARD_ILI9341_SPI    2   /* 240x320 SPI, e.g. "cheap yellow display" */
#define BOARD_GC9A01_ROUND   3   /* 240x240 round SPI, smart-watch style     */
#define BOARD_ST7796_SPI     4   /* 320x480 SPI, 3.5" modules                */
#define BOARD_RGB_800X480    5   /* 800x480 RGB parallel, 4.3"/7" ESP32-S3   */

/*  >>>>>>>>>>>>>>>>>>>>  PICK YOUR PANEL HERE  <<<<<<<<<<<<<<<<<<<<  */
#ifndef BOARD_PROFILE
#define BOARD_PROFILE  BOARD_ST7789_SPI
#endif


/* ==========================================================================
 *  Profile 1 - ST7789 over SPI, 240x320
 * ========================================================================== */
#if BOARD_PROFILE == BOARD_ST7789_SPI

#define BOARD_LCD_BUS         BUS_SPI
#define BOARD_LCD_PANEL       PANEL_ST7789
#define BOARD_LCD_H_RES       240
#define BOARD_LCD_V_RES       320
#define BOARD_LCD_INVERT      1      /* ST7789 almost always needs inversion */
#define BOARD_LCD_BGR         0      /* 1 if red and blue come out swapped   */
#define BOARD_LCD_SWAP_XY     0
#define BOARD_LCD_MIRROR_X    0
#define BOARD_LCD_MIRROR_Y    0
#define BOARD_LCD_GAP_X       0
#define BOARD_LCD_GAP_Y       0

/* ---- PINS: replace every one of these ---- */
#define BOARD_LCD_SPI_HOST    SPI2_HOST
#define BOARD_LCD_PCLK_HZ     (40 * 1000 * 1000)  /* drop to 20 MHz if noisy */
#define BOARD_PIN_LCD_SCLK    12
#define BOARD_PIN_LCD_MOSI    11
#define BOARD_PIN_LCD_DC      13
#define BOARD_PIN_LCD_CS      10
#define BOARD_PIN_LCD_RST     9      /* -1 if tied to the board reset */
#define BOARD_PIN_LCD_BL      14     /* -1 if there is no backlight pin */
#define BOARD_LCD_BL_ON_LEVEL 1      /* 0 if the backlight is active-low */

#define BOARD_TOUCH           TOUCH_NONE


/* ==========================================================================
 *  Profile 2 - ILI9341 over SPI, 240x320
 * ========================================================================== */
#elif BOARD_PROFILE == BOARD_ILI9341_SPI

#define BOARD_LCD_BUS         BUS_SPI
#define BOARD_LCD_PANEL       PANEL_ILI9341
#define BOARD_LCD_H_RES       240
#define BOARD_LCD_V_RES       320
#define BOARD_LCD_INVERT      0
#define BOARD_LCD_BGR         1      /* ILI9341 modules are usually BGR */
#define BOARD_LCD_SWAP_XY     0
#define BOARD_LCD_MIRROR_X    0
#define BOARD_LCD_MIRROR_Y    0
#define BOARD_LCD_GAP_X       0
#define BOARD_LCD_GAP_Y       0

/* ---- PINS: replace every one of these ---- */
#define BOARD_LCD_SPI_HOST    SPI2_HOST
#define BOARD_LCD_PCLK_HZ     (40 * 1000 * 1000)
#define BOARD_PIN_LCD_SCLK    14
#define BOARD_PIN_LCD_MOSI    13
#define BOARD_PIN_LCD_DC      2
#define BOARD_PIN_LCD_CS      15
#define BOARD_PIN_LCD_RST     -1
#define BOARD_PIN_LCD_BL      21
#define BOARD_LCD_BL_ON_LEVEL 1

#define BOARD_TOUCH           TOUCH_NONE


/* ==========================================================================
 *  Profile 3 - GC9A01 round panel over SPI, 240x240
 * ========================================================================== */
#elif BOARD_PROFILE == BOARD_GC9A01_ROUND

#define BOARD_LCD_BUS         BUS_SPI
#define BOARD_LCD_PANEL       PANEL_GC9A01
#define BOARD_LCD_H_RES       240
#define BOARD_LCD_V_RES       240
#define BOARD_LCD_INVERT      1
#define BOARD_LCD_BGR         1
#define BOARD_LCD_SWAP_XY     0
#define BOARD_LCD_MIRROR_X    0
#define BOARD_LCD_MIRROR_Y    0
#define BOARD_LCD_GAP_X       0
#define BOARD_LCD_GAP_Y       0

/* ---- PINS: replace every one of these ---- */
#define BOARD_LCD_SPI_HOST    SPI2_HOST
#define BOARD_LCD_PCLK_HZ     (40 * 1000 * 1000)
#define BOARD_PIN_LCD_SCLK    10
#define BOARD_PIN_LCD_MOSI    11
#define BOARD_PIN_LCD_DC      8
#define BOARD_PIN_LCD_CS      9
#define BOARD_PIN_LCD_RST     14
#define BOARD_PIN_LCD_BL      2
#define BOARD_LCD_BL_ON_LEVEL 1

#define BOARD_TOUCH           TOUCH_CST816S


/* ==========================================================================
 *  Profile 4 - ST7796 over SPI, 320x480
 * ========================================================================== */
#elif BOARD_PROFILE == BOARD_ST7796_SPI

#define BOARD_LCD_BUS         BUS_SPI
#define BOARD_LCD_PANEL       PANEL_ST7796
#define BOARD_LCD_H_RES       320
#define BOARD_LCD_V_RES       480
#define BOARD_LCD_INVERT      0
#define BOARD_LCD_BGR         1
#define BOARD_LCD_SWAP_XY     0
#define BOARD_LCD_MIRROR_X    0
#define BOARD_LCD_MIRROR_Y    0
#define BOARD_LCD_GAP_X       0
#define BOARD_LCD_GAP_Y       0

/* ---- PINS: replace every one of these ---- */
#define BOARD_LCD_SPI_HOST    SPI2_HOST
#define BOARD_LCD_PCLK_HZ     (40 * 1000 * 1000)
#define BOARD_PIN_LCD_SCLK    12
#define BOARD_PIN_LCD_MOSI    11
#define BOARD_PIN_LCD_DC      13
#define BOARD_PIN_LCD_CS      10
#define BOARD_PIN_LCD_RST     9
#define BOARD_PIN_LCD_BL      14
#define BOARD_LCD_BL_ON_LEVEL 1

#define BOARD_TOUCH           TOUCH_FT5X06


/* ==========================================================================
 *  Profile 5 - 16-bit RGB parallel panel, 800x480  (ESP32-S3 + octal PSRAM)
 *
 *  Needs PSRAM enabled in sdkconfig.defaults - see HANDOVER.md.
 * ========================================================================== */
#elif BOARD_PROFILE == BOARD_RGB_800X480

#define BOARD_LCD_BUS         BUS_RGB
#define BOARD_LCD_PANEL       PANEL_RGB
#define BOARD_LCD_H_RES       800
#define BOARD_LCD_V_RES       480
#define BOARD_LCD_SWAP_XY     0
#define BOARD_LCD_MIRROR_X    0
#define BOARD_LCD_MIRROR_Y    0

/* ---- Panel timings: from your panel's datasheet ---- */
#define BOARD_RGB_PCLK_HZ           (16 * 1000 * 1000)
#define BOARD_RGB_HSYNC_BACK_PORCH  8
#define BOARD_RGB_HSYNC_FRONT_PORCH 8
#define BOARD_RGB_HSYNC_PULSE_WIDTH 4
#define BOARD_RGB_VSYNC_BACK_PORCH  16
#define BOARD_RGB_VSYNC_FRONT_PORCH 16
#define BOARD_RGB_VSYNC_PULSE_WIDTH 4
#define BOARD_RGB_PCLK_ACTIVE_NEG   1

/* ---- PINS: replace every one of these ---- */
#define BOARD_PIN_RGB_PCLK    7
#define BOARD_PIN_RGB_VSYNC   3
#define BOARD_PIN_RGB_HSYNC   46
#define BOARD_PIN_RGB_DE      5
#define BOARD_PIN_RGB_DISP    -1
#define BOARD_PIN_LCD_BL      2
#define BOARD_LCD_BL_ON_LEVEL 1

/* 16 data lines: B0..B4, G0..G5, R0..R4 (that order is not a typo) */
#define BOARD_RGB_DATA_GPIOS  { 14, 38, 18, 17, 10, \
                                39, 0, 45, 48, 47, 21, \
                                1, 2, 42, 41, 40 }

#define BOARD_TOUCH           TOUCH_GT911

#else
#error "BOARD_PROFILE is not set to a known profile - see board_config.h"
#endif


/* ==========================================================================
 *  Touch wiring (all supported controllers are I2C)
 * ========================================================================== */
#if BOARD_TOUCH != TOUCH_NONE
#define BOARD_TOUCH_I2C_PORT  0
#define BOARD_TOUCH_I2C_HZ    400000
#define BOARD_PIN_TOUCH_SDA   6
#define BOARD_PIN_TOUCH_SCL   7
#define BOARD_PIN_TOUCH_RST   -1
#define BOARD_PIN_TOUCH_INT   -1
#endif
