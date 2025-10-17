#ifndef SCREENS_H
#define SCREENS_H

#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "src/comm/socket_driver.h"
#include "jsmn.h"


void main_screen(void);
void menu_screen(void);
void list_screen(void);
void register_screen(void);
void delete_screen(void);

#endif
