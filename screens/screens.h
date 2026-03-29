#ifndef SCREENS_H
#define SCREENS_H

#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "socket_driver.h"
#define JSMN_STATIC
#include "jsmn.h"

#define DISPLAY_WIDTH "480"
#define DISPLAY_HEIGHT "320"

#define UI_COLOR_SCREEN_BG 0x00181818
#define UI_COLOR_SCREEN_BORDER 0x000D0D0D

#define UI_COLOR_HEADER_BG_LIGHT 0x00A3A3A3
#define UI_COLOR_HEADER_BORDER_LIGHT 0x00F2F2F2
#define UI_COLOR_HEADER_TEXT_LIGHT 0x00FFFFFF

#define UI_COLOR_HEADER_BG_DARK 0x005C5C5C
#define UI_COLOR_HEADER_BORDER_DARK 0x000D0D0D
#define UI_COLOR_HEADER_TEXT_DARK 0x00000000

#define UI_COLOR_ACCENT_BLUE 0x002196F3
#define UI_COLOR_ACCENT_ORANGE 0x00DE690C
#define UI_COLOR_ACCENT_RED 0x00FF0000
#define UI_COLOR_TEXT_WHITE 0x00FFFFFF
#define UI_COLOR_TEXT_BLACK 0x00000000

void main_screen(void);
void menu_screen(void);
void list_screen(void);
void register_screen(void);
void delete_screen(void);

#endif
