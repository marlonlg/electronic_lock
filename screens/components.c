#include "screens.h"

/*******************************************************
 * Create a header with title and return button
 *******************************************************/

void (*return_screen)(void) = 0;

static void event_handler_cb_scr_list_btn_return_3(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_RELEASED) {
        lv_obj_clean(lv_scr_act());
        return_screen();
    }
}

void create_header(lv_obj_t *parent_obj, const char *title, void (*return_callback)(void)) {
    lv_obj_t *obj = lv_obj_create(parent_obj);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(10));
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_HEADER_BG_DARK), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, lv_color_hex(UI_COLOR_HEADER_BORDER_DARK), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 0, 1);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, title);
        }
        {
            if (return_callback != NULL) {
                return_screen = return_callback;
                
                lv_obj_t *obj = lv_button_create(parent_obj);
                lv_obj_set_pos(obj, 0, 0);
                lv_obj_set_size(obj, 40, 35);
                lv_obj_add_event_cb(obj, event_handler_cb_scr_list_btn_return_3, LV_EVENT_ALL, 0);
                lv_obj_set_style_align(obj, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_ITEMS | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    lv_obj_t *parent_obj = obj;
                    {
                        lv_obj_t *obj = lv_label_create(parent_obj);
                        lv_obj_set_pos(obj, 0, 0);
                        lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(obj, "<");
                    }
                }
            }
        }
    }
}