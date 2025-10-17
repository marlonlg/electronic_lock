#include <stddef.h>
#include "screens.h"


typedef struct _objects_t {
    lv_obj_t *scr_register;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *btn_register_1;
    lv_obj_t *txt_reg_name_1;
    lv_obj_t *txt_password_1;
    lv_obj_t *txt_tag_data_1;
    lv_obj_t *btn_return_1;
} objects_t;

// Make register_objects global so it can be accessed in all functions
static objects_t register_objects;

// --- Controle global do teclado ---
static lv_obj_t *g_keyboard = NULL;

static void show_hide_keyboard_cb(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    if(lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        if(g_keyboard) {
            lv_obj_clear_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
            lv_keyboard_set_textarea(g_keyboard, ta);
        }
    }
    if(lv_event_get_code(e) == LV_EVENT_DEFOCUSED) {
        if(g_keyboard) {
            lv_obj_add_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void event_handler_cb_scr_register_btn_return_1(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_RELEASED) {
        lv_obj_clean(lv_scr_act());
        menu_screen();
    }
}

static void event_handler_cb_scr_register_btn_register(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_RELEASED) {
        printf("Register button clicked\n");
        char send_buf[256];
        snprintf(send_buf, sizeof(send_buf), "{\"cmd\":\"add_user\",\"usuario\":\"%s\",\"senha\":\"%s\",\"tag_rfid\":\"%s\"}",
                 lv_textarea_get_text(register_objects.txt_reg_name_1),
                 lv_textarea_get_text(register_objects.txt_password_1),
                 lv_textarea_get_text(register_objects.txt_tag_data_1));
        lvgl_socket_send(send_buf);
        printf("Sent: %s\n", send_buf);
        lv_textarea_set_text(register_objects.txt_reg_name_1, "");
        lv_textarea_set_text(register_objects.txt_password_1, "");
        lv_textarea_set_text(register_objects.txt_tag_data_1, "");
    }
}

// Callback para configurar o teclado quando um campo de texto recebe o foco
static void event_handler_textarea_focus(lv_event_t *e) {
    lv_obj_t *textarea = lv_event_get_target(e);
    lv_obj_t *keyboard = lv_event_get_user_data(e);

    if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(keyboard, textarea);
    }
}

void register_screen(void){
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    register_objects.scr_register = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 320, 480);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    register_objects.obj3 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(10));
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa3a3a3), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xfff2f2f2), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            register_objects.obj4 = obj;
                            lv_obj_set_pos(obj, 0, 1);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Cadastro");
                        }
                        {
                            // btn_return_1
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            register_objects.btn_return_1 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 40, 35);
                            lv_obj_add_event_cb(obj, event_handler_cb_scr_register_btn_return_1, LV_EVENT_ALL, 0);
                            lv_obj_set_style_align(obj, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "<");
                                }
                            }
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(0));
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(90));
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                    lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 11);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Nome:");
                        }
                        {
                            // txt_reg_name_1
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            register_objects.txt_reg_name_1 = obj;
                            lv_obj_set_pos(obj, 0, LV_PCT(0));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 16);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_state(obj, LV_STATE_FOCUSED);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(16));
                            lv_obj_set_size(obj, 49, 16);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Senha:");
                        }
                        {
                            // txt_password_1
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            register_objects.txt_password_1 = obj;
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(13));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, true);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(28));
                            lv_obj_set_size(obj, 63, 16);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Tag:");
                        }
                        {
                            // txt_tag_data_1
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            register_objects.txt_tag_data_1 = obj;
                            lv_obj_set_pos(obj, 0, LV_PCT(25));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            // btn_register_1
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            register_objects.btn_register_1 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 117, 50);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_add_event_cb(obj, event_handler_cb_scr_register_btn_register, LV_EVENT_RELEASED, NULL);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Cadastrar");
                                }
                            }
                        }
                        {
                            lv_obj_t *obj = lv_keyboard_create(parent_obj);
                            register_objects.obj5 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(110), 120);
                            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_TEXT_UPPER);
                            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
                            lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }
    }
    g_keyboard = lv_keyboard_create(obj);
    lv_obj_add_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(g_keyboard, 0, 0);
    lv_obj_set_size(g_keyboard, LV_PCT(110), 120);

    lv_keyboard_set_textarea(register_objects.obj5, register_objects.txt_reg_name_1);
    // Cria um teclado virtual
    // lv_obj_t *keyboard = lv_keyboard_create(obj);
    // lv_obj_set_pos(keyboard, 0, 0);
    // lv_obj_set_size(keyboard, LV_PCT(110), 120);

    // Adiciona o evento de foco para os campos de texto
    lv_obj_add_event_cb(register_objects.txt_reg_name_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    lv_obj_add_event_cb(register_objects.txt_password_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    lv_obj_add_event_cb(register_objects.txt_tag_data_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    // Adiciona eventos para mostrar/ocultar teclado
    lv_obj_add_event_cb(register_objects.txt_reg_name_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(register_objects.txt_password_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(register_objects.txt_tag_data_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);
}

// void register_screen(void){

//     lv_obj_t *obj = lv_obj_create(lv_screen_active());
//     register_objects.scr_register = obj;

//     lv_obj_set_pos(obj, 0, 0);
//     lv_obj_set_size(obj, 320, 480);
//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
//     {
//         lv_obj_t *parent_obj = obj;
//         {
//             lv_obj_t *obj = lv_obj_create(parent_obj);
//             lv_obj_set_pos(obj, 0, 0);
//             lv_obj_set_size(obj, 320, 480);
//             lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             {
//                 lv_obj_t *parent_obj = obj;
//                 {
//                     lv_obj_t *obj = lv_obj_create(parent_obj);
//                     lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(13));
//                     lv_obj_set_size(obj, 470, 270);
//                     lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
//                     lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
//                     {
//                         lv_obj_t *parent_obj = obj;
//                         {
//                             lv_obj_t *obj = lv_label_create(parent_obj);
//                             lv_obj_set_pos(obj, 0, 0);
//                             lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//                             lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_label_set_text(obj, "Nome:");
//                         }
//                         {
//                             // txt_reg_name_1
//                             lv_obj_t *obj = lv_textarea_create(parent_obj);
//                             register_objects.txt_reg_name_1 = obj;
//                             lv_obj_set_pos(obj, 0, LV_PCT(10));
//                             lv_obj_set_size(obj, 166, 40);
//                             lv_textarea_set_max_length(obj, 16);
//                             lv_textarea_set_one_line(obj, true);
//                             lv_textarea_set_password_mode(obj, false);
//                             lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             // Define o foco no campo de texto txt_reg_name_1
//                             lv_obj_add_state(register_objects.txt_reg_name_1, LV_STATE_FOCUSED);
//                             //lv_keyboard_set_textarea(NULL, register_objects.txt_reg_name_1);
//                         }
//                         {
//                             lv_obj_t *obj = lv_label_create(parent_obj);
//                             lv_obj_set_pos(obj, LV_PCT(-27), 0);
//                             lv_obj_set_size(obj, 49, 16);
//                             lv_obj_set_style_align(obj, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_label_set_text(obj, "Senha:");
//                         }
//                         {
//                             // txt_password_1
//                             lv_obj_t *obj = lv_textarea_create(parent_obj);
//                             register_objects.txt_password_1 = obj;
//                             lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(10));
//                             lv_obj_set_size(obj, 166, 40);
//                             lv_textarea_set_max_length(obj, 128);
//                             lv_textarea_set_one_line(obj, true);
//                             lv_textarea_set_password_mode(obj, true);
//                             lv_obj_set_style_align(obj, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
//                         }
//                         {
//                             lv_obj_t *obj = lv_label_create(parent_obj);
//                             lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(31));
//                             lv_obj_set_size(obj, 63, 16);
//                             lv_label_set_text(obj, "TagData:");
//                         }
//                         {
//                             // txt_tag_data_1
//                             lv_obj_t *obj = lv_textarea_create(parent_obj);
//                             register_objects.txt_tag_data_1 = obj;
//                             lv_obj_set_pos(obj, 0, LV_PCT(40));
//                             lv_obj_set_size(obj, 165, 40);
//                             lv_textarea_set_max_length(obj, 128);
//                             lv_textarea_set_one_line(obj, true);
//                             lv_textarea_set_password_mode(obj, false);
//                             lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
//                         }
//                         {
//                             // btn_register_1
//                             lv_obj_t *obj = lv_button_create(parent_obj);
//                             register_objects.btn_register_1 = obj;
//                             lv_obj_set_pos(obj, 0, 0);
//                             lv_obj_set_size(obj, 117, 50);
//                             lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_obj_add_event_cb(obj, event_handler_cb_scr_register_btn_register, LV_EVENT_RELEASED, NULL);
//                             lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             {
//                                 lv_obj_t *parent_obj = obj;
//                                 {
//                                     lv_obj_t *obj = lv_label_create(parent_obj);
//                                     lv_obj_set_pos(obj, 0, 0);
//                                     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//                                     lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
//                                     lv_label_set_text(obj, "Cadastrar");
//                                 }
//                             }
//                         }
//                     }
//                 }
//                 {
//                     lv_obj_t *obj = lv_obj_create(parent_obj);
//                     register_objects.obj2 = obj;
//                     lv_obj_set_pos(obj, 0, 0);
//                     lv_obj_set_size(obj, LV_PCT(100), 40);
//                     lv_obj_set_style_bg_color(obj, lv_color_hex(0x005C5C5C), LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_border_color(obj, lv_color_hex(0x000D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
//                     {
//                         lv_obj_t *parent_obj = obj;
//                         {
//                             lv_obj_t *obj = lv_label_create(parent_obj);
//                             register_objects.obj3 = obj;
//                             lv_obj_set_pos(obj, 0, 1);
//                             lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//                             lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_obj_set_style_text_color(obj, lv_color_hex(0x00000000), LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             lv_label_set_text(obj, "Home");
//                         }
//                         {
//                             // btn_return_1
//                             lv_obj_t *obj = lv_button_create(parent_obj);
//                             register_objects.btn_return_1 = obj;
//                             lv_obj_set_pos(obj, 0, 0);
//                             lv_obj_set_size(obj, 40, 30);
//                             lv_obj_add_event_cb(obj, event_handler_cb_scr_register_btn_return_1, LV_EVENT_ALL, 0);
//                             lv_obj_set_style_align(obj, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
//                             {
//                                 lv_obj_t *parent_obj = obj;
//                                 {
//                                     lv_obj_t *obj = lv_label_create(parent_obj);
//                                     lv_obj_set_pos(obj, 0, 0);
//                                     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//                                     lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
//                                     lv_obj_set_style_text_font(obj, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
//                                     lv_label_set_text(obj, "<");
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
//     // Cria um teclado virtual
//     lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());

//     // Adiciona o evento de foco para os campos de texto
//     lv_obj_add_event_cb(register_objects.txt_reg_name_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, keyboard);
//     lv_obj_add_event_cb(register_objects.txt_password_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, keyboard);
//     lv_obj_add_event_cb(register_objects.txt_tag_data_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, keyboard);
// }