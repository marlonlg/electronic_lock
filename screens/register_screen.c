#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "screens.h"

#define REGISTER_SCREEN_POLL_INTERVAL_MS 500
#define REGISTER_SCREEN_RECV_TIMEOUT_MS 50
#define REGISTER_SCREEN_POLL_CMD "{\"cmd\":\"read_register_tag\"}"
#define REGISTER_SCREEN_MAX_JSON_TOKENS 32
#define REGISTER_SCREEN_MAX_JSON_VALUE 256


typedef struct _objects_t {
    lv_obj_t *scr_register;
    lv_obj_t *obj5;
    lv_obj_t *btn_register_1;
    lv_obj_t *btn_add_photo_1;
    lv_obj_t *img_photo_preview_1;
    lv_obj_t *lbl_add_photo_1;
    lv_obj_t *txt_reg_name_1;
    lv_obj_t *txt_password_1;
    lv_obj_t *txt_tag_data_1;
} objects_t;

typedef struct {
    lv_obj_t * file_explorer;
} fe_ctx_t;

// Make register_objects global so it can be accessed in all functions
static objects_t register_objects;
static lv_timer_t *g_register_poll_timer = NULL;

// --- Controle global do teclado ---
static lv_obj_t *g_keyboard = NULL;
static lv_obj_t *g_photo_picker_modal = NULL;
static char g_selected_photo_path[LV_FS_MAX_PATH_LENGTH] = "";

static void file_explorer_event_handler(lv_event_t * e);
static void register_poll_timer_cb(lv_timer_t *timer);
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
static bool json_get_string(const char *json, jsmntok_t *tokens, int token_count, const char *key, char *out, size_t out_size);

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

static void event_handler_cb_scr_register_btn_register(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_RELEASED) {
        printf("Register button clicked\n");
        char send_buf[256];
        snprintf(send_buf, sizeof(send_buf), "{\"cmd\":\"add_user\",\"usuario\":\"%s\",\"senha\":\"%s\",\"tag_rfid\":\"%s\",\"img_path\":\"%s\"}",
                 lv_textarea_get_text(register_objects.txt_reg_name_1),
                 lv_textarea_get_text(register_objects.txt_password_1),
                 lv_textarea_get_text(register_objects.txt_tag_data_1),
                 g_selected_photo_path);
        lvgl_socket_send(send_buf);
        printf("Sent: %s\n", send_buf);
        lv_textarea_set_text(register_objects.txt_reg_name_1, "");
        lv_textarea_set_text(register_objects.txt_password_1, "");
        lv_textarea_set_text(register_objects.txt_tag_data_1, "");
        g_selected_photo_path[0]= '\0';
    }
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }

    return -1;
}

static bool json_get_string(const char *json, jsmntok_t *tokens, int token_count, const char *key, char *out, size_t out_size) {
    if (!out || out_size == 0) return false;

    for (int i = 1; i < token_count - 1; i++) {
        if (jsoneq(json, &tokens[i], key) == 0) {
            int len = tokens[i + 1].end - tokens[i + 1].start;
            if (len < 0) return false;
            if ((size_t)len >= out_size) len = (int)out_size - 1;
            memcpy(out, json + tokens[i + 1].start, (size_t)len);
            out[len] = '\0';
            return true;
        }
    }

    return false;
}

static void register_poll_timer_cb(lv_timer_t *timer) {
    char recv_buf[512];
    jsmn_parser parser;
    jsmntok_t tokens[REGISTER_SCREEN_MAX_JSON_TOKENS];
    char cmd[REGISTER_SCREEN_MAX_JSON_VALUE];
    char tag_rfid[REGISTER_SCREEN_MAX_JSON_VALUE];

    LV_UNUSED(timer);

    if (!register_objects.txt_tag_data_1) return;

    if (lvgl_socket_send(REGISTER_SCREEN_POLL_CMD) < 0) return;
    if (lvgl_socket_recv_timeout(recv_buf, sizeof(recv_buf), REGISTER_SCREEN_RECV_TIMEOUT_MS) < 0) return;

    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, recv_buf, strlen(recv_buf), tokens, sizeof(tokens) / sizeof(tokens[0]));
    if (token_count <= 0) return;

    if (!json_get_string(recv_buf, tokens, token_count, "cmd", cmd, sizeof(cmd))) return;
    if (strcmp(cmd, "tag_read") != 0) return;

    if (!json_get_string(recv_buf, tokens, token_count, "tag_rfid", tag_rfid, sizeof(tag_rfid)) &&
        !json_get_string(recv_buf, tokens, token_count, "tag", tag_rfid, sizeof(tag_rfid))) {
        return;
    }

    lv_textarea_set_text(register_objects.txt_tag_data_1, tag_rfid);
}

static void close_btn_event_cb(lv_event_t * e)
{
    fe_ctx_t * ctx = lv_event_get_user_data(e);
    if(ctx && ctx->file_explorer) {
        if(ctx->file_explorer == g_photo_picker_modal) {
            g_photo_picker_modal = NULL;
        }
        lv_obj_delete(ctx->file_explorer);
    }
}

static void event_handler_cb_scr_add_photo(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_RELEASED) {
        if(g_photo_picker_modal) {
            lv_obj_delete(g_photo_picker_modal);
            g_photo_picker_modal = NULL;
        }
        
        g_photo_picker_modal = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(g_photo_picker_modal);
        lv_obj_set_size(g_photo_picker_modal, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(g_photo_picker_modal, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(g_photo_picker_modal, LV_OPA_60, 0);
        lv_obj_add_flag(g_photo_picker_modal, LV_OBJ_FLAG_CLICKABLE);
        
        lv_obj_t * panel = lv_obj_create(g_photo_picker_modal);
        lv_obj_set_size(panel, LV_PCT(90), LV_PCT(80));
        lv_obj_center(panel);
        lv_obj_set_style_pad_all(panel, 8, 0);
        
        lv_obj_t * file_explorer = lv_file_explorer_create(panel);
        static fe_ctx_t ctx;
        ctx.file_explorer = g_photo_picker_modal;
        lv_obj_set_size(file_explorer, LV_PCT(100), LV_PCT(100));
        lv_file_explorer_show_back_button(file_explorer, false);
        lv_file_explorer_set_sort(file_explorer, LV_EXPLORER_SORT_NONE);
        lv_file_explorer_open_dir(file_explorer, "A:/home/marlon/Downloads");
        lv_obj_add_event_cb(file_explorer, file_explorer_event_handler, LV_EVENT_ALL, NULL);

        lv_obj_t * header = lv_file_explorer_get_header(file_explorer);
        lv_obj_t * btn = lv_button_create(header);
        lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -4, 0);
        lv_obj_set_size(btn, 40, 32);
        lv_obj_add_event_cb(btn, close_btn_event_cb, LV_EVENT_CLICKED, &ctx);

        lv_obj_t * lbl = lv_label_create(btn);
        lv_label_set_text(lbl, "X");
        lv_obj_center(lbl);
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


static void close_image_preview_event_handler(lv_event_t * e)
{
    lv_obj_t * modal = lv_event_get_user_data(e);
    if(modal) lv_obj_delete(modal);
}

static bool is_png_file_name(const char * file_name)
{
    const char * ext = strrchr(file_name, '.');
    if(ext == NULL) return false;

    return (tolower((unsigned char)ext[1]) == 'p') &&
           (tolower((unsigned char)ext[2]) == 'n') &&
           (tolower((unsigned char)ext[3]) == 'g') &&
           (ext[4] == '\0');
}

static void open_image_preview(const char * file_path)
{
    lv_obj_t * modal = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(modal);
    lv_obj_set_size(modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(modal, LV_OPA_70, 0);
    lv_obj_add_flag(modal, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(modal, close_image_preview_event_handler, LV_EVENT_CLICKED, modal);

    lv_obj_t * panel = lv_obj_create(modal);
    lv_obj_set_size(panel, LV_PCT(75), LV_PCT(75));
    lv_obj_center(panel);
    lv_obj_set_style_pad_all(panel, 10, 0);
    lv_obj_add_event_cb(panel, lv_event_stop_bubbling, LV_EVENT_CLICKED, NULL);

    lv_obj_t * close_btn = lv_button_create(panel);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_add_event_cb(close_btn, close_image_preview_event_handler, LV_EVENT_CLICKED, modal);

    lv_obj_t * close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
    lv_obj_center(close_label);

    lv_obj_t * img = lv_image_create(panel);
    lv_obj_set_size(img, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_top(img, 32, 0);
    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CONTAIN);
    lv_image_set_src(img, file_path);
}

static void exch_table_item(lv_obj_t * tb, int16_t i, int16_t j)
{
    const char * tmp;
    tmp = lv_table_get_cell_value(tb, i, 0);
    lv_table_set_cell_value(tb, 0, 2, tmp);
    lv_table_set_cell_value(tb, i, 0, lv_table_get_cell_value(tb, j, 0));
    lv_table_set_cell_value(tb, j, 0, lv_table_get_cell_value(tb, 0, 2));

    tmp = lv_table_get_cell_value(tb, i, 1);
    lv_table_set_cell_value(tb, 0, 2, tmp);
    lv_table_set_cell_value(tb, i, 1, lv_table_get_cell_value(tb, j, 1));
    lv_table_set_cell_value(tb, j, 1, lv_table_get_cell_value(tb, 0, 2));
}

/*Quick sort 3 way*/
static void sort_by_file_kind(lv_obj_t * tb, int16_t lo, int16_t hi)
{
    if(lo >= hi) return;

    int16_t lt = lo;
    int16_t i = lo + 1;
    int16_t gt = hi;
    const char * v = lv_table_get_cell_value(tb, lo, 1);
    while(i <= gt) {
        if(strcmp(lv_table_get_cell_value(tb, i, 1), v) < 0)
            exch_table_item(tb, lt++, i++);
        else if(strcmp(lv_table_get_cell_value(tb, i, 1), v) > 0)
            exch_table_item(tb, i, gt--);
        else
            i++;
    }

    sort_by_file_kind(tb, lo, lt - 1);
    sort_by_file_kind(tb, gt + 1, hi);
}

static void file_explorer_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        const char * cur_path =  lv_file_explorer_get_current_path(obj);
        const char * sel_fn = lv_file_explorer_get_selected_file_name(obj);

        LV_LOG_USER("%s%s", cur_path, sel_fn);

        if(is_png_file_name(sel_fn)) {
            char file_path[LV_FS_MAX_PATH_LENGTH];
            lv_snprintf(file_path, sizeof(file_path), "%s%s", cur_path, sel_fn);
            lv_snprintf(g_selected_photo_path, sizeof(g_selected_photo_path), "%s", file_path);
            if(register_objects.img_photo_preview_1) {
                open_image_preview(g_selected_photo_path);
                lv_image_set_src(register_objects.img_photo_preview_1, g_selected_photo_path);
            }
            if(register_objects.lbl_add_photo_1) {
                lv_obj_add_flag(register_objects.lbl_add_photo_1, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    else if(code == LV_EVENT_READY) {
        lv_obj_t * tb = lv_file_explorer_get_file_table(obj);
        uint16_t sum = lv_table_get_row_count(tb);

        sort_by_file_kind(tb, 0, (sum - 1));
    }
}

void lv_example_file_explorer(void)
{
    lv_obj_t * file_explorer = lv_file_explorer_create(lv_screen_active());
    lv_file_explorer_show_back_button(file_explorer, false);
    /*Before custom sort, please set the default sorting to NONE. The default is NONE.*/
    lv_file_explorer_set_sort(file_explorer, LV_EXPLORER_SORT_NONE);

    lv_file_explorer_open_dir(file_explorer, "A:/home/marlon/Downloads");



    lv_obj_add_event_cb(file_explorer, file_explorer_event_handler, LV_EVENT_ALL, NULL);
}


void register_screen(void){
    if (g_register_poll_timer) {
        lv_timer_del(g_register_poll_timer);
        g_register_poll_timer = NULL;
    }

    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    register_objects.scr_register = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                create_header(parent_obj, "Cadastro", menu_screen);
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
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(4));
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Nome:");
                        }
                        {
                            // txt_reg_name_1
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            register_objects.txt_reg_name_1 = obj;
                            lv_obj_set_pos(obj, LV_PCT(13), LV_PCT(0));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 16);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_state(obj, LV_STATE_FOCUSED);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_HEADER_BG_DARK), LV_PART_MAIN | LV_STATE_DEFAULT);
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
                            lv_obj_set_pos(obj, LV_PCT(13), LV_PCT(13));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, true);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_HEADER_BG_DARK), LV_PART_MAIN | LV_STATE_DEFAULT);
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
                            lv_obj_set_pos(obj, LV_PCT(13), LV_PCT(25));
                            lv_obj_set_size(obj, 200, 35);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_HEADER_BG_DARK), LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(40));
                            lv_obj_set_size(obj, 63, 16);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Foto:");
                        }
                        {
                            // btn_register_1
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            register_objects.btn_add_photo_1 = obj;
                            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(42));
                            lv_obj_set_size(obj, 120, 120);
                            lv_obj_set_style_align(obj, LV_ALIGN_TOP_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_add_event_cb(obj, event_handler_cb_scr_add_photo, LV_EVENT_RELEASED, NULL);
                            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    register_objects.img_photo_preview_1 = obj;
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_image_set_inner_align(obj, LV_IMAGE_ALIGN_CONTAIN);
                                }
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    register_objects.lbl_add_photo_1 = obj;
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "Selecionar");
                                }
                            }
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
                            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_size(g_keyboard, LV_PCT(100), LV_PCT(50));

    lv_keyboard_set_textarea(register_objects.obj5, register_objects.txt_reg_name_1);

    // Adiciona o evento de foco para os campos de texto
    lv_obj_add_event_cb(register_objects.txt_reg_name_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    lv_obj_add_event_cb(register_objects.txt_password_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    lv_obj_add_event_cb(register_objects.txt_tag_data_1, event_handler_textarea_focus, LV_EVENT_FOCUSED, g_keyboard);
    // Adiciona eventos para mostrar/ocultar teclado
    lv_obj_add_event_cb(register_objects.txt_reg_name_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(register_objects.txt_password_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(register_objects.txt_tag_data_1, show_hide_keyboard_cb, LV_EVENT_ALL, NULL);

    g_register_poll_timer = lv_timer_create(register_poll_timer_cb, REGISTER_SCREEN_POLL_INTERVAL_MS, NULL);
}

