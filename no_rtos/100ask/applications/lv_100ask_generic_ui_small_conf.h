/**
 * @file lv_100ask_generic_ui_small_conf.h
 * Configuration file for v9.2.0
 *
 */
/*
 * COPY THIS FILE AS lv_100ask_generic_ui_small_conf.h
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable the content*/

#ifndef LV_100ASK_GENERIC_UI_SMALLE_CONF_H
#define LV_100ASK_GENERIC_UI_SMALLE_CONF_H

#include "lv_lib_100ask/lv_lib_100ask.h"

/*******************
 * GENERAL SETTING
 *******************/
/* platform */
#define LV_100ASK_DESKTOP_USE_SIMULATOR                 1
#define LV_100ASK_DESKTOP_USE_DshanMCUH7R_NoRTOS        0

/* screen size */
#define LV_100ASK_SCREEN_SIZE_320X480                   1
#define LV_100ASK_SCREEN_SIZE_480X480                   0

/* other */
#define LV_100ASK_HAS_DESKTOP_BG                        0
#if LV_100ASK_HAS_DESKTOP_BG
#define LV_100ASK_LIMIT_DESKTOP_BG_PIC                  1
#endif

#define LV_100ASK_HAS_LOCK_SCREEN                       0
#if LV_100ASK_HAS_LOCK_SCREEN
#define LV_100ASK_LIMIT_LOCK_SCREEN_PIC                 1
#endif

#define LCD_MAX_BACKLIGHT                               1000
#define LCD_MIN_BACKLIGHT                               200

#endif /*LV_100ASK_GENERIC_UI_SMALLE_CONF_H*/

#endif /*End of "Content enable"*/

