#include "screens.h"
#include "jsmn.h"

typedef struct _objects_t {
    lv_obj_t *scr_list;
    lv_obj_t *tbl_users;
} objects_t;

static objects_t list_objects;

static void event_handler_cb_scr_list_btn_return_3(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_RELEASED) {
        lv_obj_clean(lv_scr_act());
        menu_screen();
    }
}

static void draw_event_cb(lv_event_t * e)
{
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
    /*If the cells are drawn...*/
    if(base_dsc->part == LV_PART_ITEMS) {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        /*Make the texts in the first cell center aligned*/
        if(row == 0) {
            lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if(label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
            }
            lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if(fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), fill_draw_dsc->color, LV_OPA_20);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
        /*In the first column align the texts to the right*/
        else if(col == 0) {
            lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if(label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_LEFT;
            }
        }

        /*Make every 2nd row grayish*/
        if((row != 0 && row % 2) == 0) {
            lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if(fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fill_draw_dsc->color, LV_OPA_10);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
    }
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

static const char* get_json_value(const char *json, jsmntok_t *tokens, int token_count, const char *key) {
    for (int i = 0; i < token_count; i++) {
        if (jsoneq(json, &tokens[i], key) == 0) {
            return json + tokens[i + 1].start; // Retorna o início do valor
        }
    }
    return NULL; // Retorna NULL se a chave não for encontrada
}

void list_screen(void){
    static jsmn_parser p;
    static jsmntok_t t[128]; /* We expect no more than 128 tokens*/
    jsmn_init(&p);

    lv_obj_clean(lv_scr_act());
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    list_objects.scr_list = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            create_header(parent_obj, "Usuarios", menu_screen);
        }
        {
            // tbl_users
            lv_obj_t *obj = lv_table_create(parent_obj);
            list_objects.tbl_users = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_ON);
            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_t * table = lv_table_create(lv_screen_active());

            /*Fill the first column*/
            lv_table_set_cell_value(obj, 0, 0, "Usuario");
            // lv_table_set_cell_value(obj, 0, 1, "senha");
            // lv_table_set_cell_value(obj, 0, 2, "tag_rfid");



            lvgl_socket_send("{\"cmd\":\"list_users\"}");
            char recv_buf[1024]; // Buffer maior para lidar com listas maiores

            if (lvgl_socket_recv(recv_buf, sizeof(recv_buf)) >= 0) {
                printf("Received: %s\n", recv_buf);
                int r;
                r = jsmn_parse(&p, recv_buf, strlen(recv_buf), t, sizeof(t)/sizeof(t[0]));
                printf("Number of tokens: %d\n", r);
                if (r > 0) {
                    int row = 1; // Começa na linha 1, pois a linha 0 é o cabeçalho
                    for (int i = 0; i < r; i++) {
                        if (t[i].type == JSMN_OBJECT) {
                            const char *usuario = get_json_value(recv_buf, t, r, "usuario");
                            // const char *senha = get_json_value(recv_buf, t, r, "senha");
                            // const char *tag_rfid = get_json_value(recv_buf, t, r, "tag_rfid");

                            char usuario_buf[32] = {0}, senha_buf[32] = {0}, tag_rfid_buf[32] = {0};

                            if (usuario) {
                                snprintf(usuario_buf, sizeof(usuario_buf), "%.*s", t[i + 4].end - t[i + 4].start, recv_buf + t[i + 4].start);
                                lv_table_set_cell_value(obj, row, 0, usuario_buf);
                            }
                            // if (senha) {
                            //     snprintf(senha_buf, sizeof(senha_buf), "%.*s", t[i + 6].end - t[i + 6].start, recv_buf + t[i + 6].start);
                            //     lv_table_set_cell_value(obj, row, 1, senha_buf);
                            // }
                            // if (tag_rfid) {
                            //     snprintf(tag_rfid_buf, sizeof(tag_rfid_buf), "%.*s", t[i + 8].end - t[i + 8].start, recv_buf + t[i + 8].start);
                            //     lv_table_set_cell_value(obj, row, 2, tag_rfid_buf);
                            // }

                            printf("Usuario: %s\n", usuario_buf);
                            // printf("Senha: %s\n", senha_buf);
                            // printf("Tag RFID: %s\n", tag_rfid_buf);

                            row++; // Incrementa a linha para o próximo objeto
                        }
                    }
                } else {
                    printf("Erro ao analisar o JSON ou chaves esperadas não encontradas.\n");
                }
            }
            /*Set a smaller height to the table. It'll make it scrollable*/
            lv_obj_set_height(obj, 300);
            lv_obj_center(obj);

            /*Add an event callback to to apply some custom drawing*/
            lv_obj_add_event_cb(obj, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

        }
    }
}
