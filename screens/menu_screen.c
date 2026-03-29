#include "screens.h"


typedef struct _objects_t {
    lv_obj_t *scr_menu;
    lv_obj_t *obj0;
} objects_t;

static objects_t menu_objects;


static void event_handler_cb_scr_menu_obj0(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_RELEASED) {
        uint32_t index = lv_buttonmatrix_get_selected_button(menu_objects.obj0);
        printf("Button index: %d\n", index);
        if (index == 0) {
            printf("cadastrar");
            lv_obj_clean(lv_scr_act());
            register_screen();
        }else if (index == 2) {
            printf("list_screen");
            lv_obj_clean(lv_scr_act());
            list_screen();
        }else if (index == 3) {
            lv_obj_clean(lv_scr_act());
            delete_screen();
        }
    }
}


void menu_screen(void) {
    lv_obj_clean(lv_scr_act());
    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            create_header(parent_obj, "Menu", main_screen);
        }
        {
            lv_obj_t *obj = lv_buttonmatrix_create(parent_obj);
            menu_objects.obj0 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(80));
            static const char *map[8] = {
                "Cadastrar",
                "\n",
                "Editar",
                "\n",
                "Listar",
                "\n",
                "Deletar",
                NULL,
            };
            lv_buttonmatrix_set_map(obj, map);
            lv_obj_add_event_cb(obj, event_handler_cb_scr_menu_obj0, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

    }
}
