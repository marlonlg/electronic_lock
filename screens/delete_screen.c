#include "screens.h"

typedef struct _objects_t {
    lv_obj_t *scr_list;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *btn_return_3;
    lv_obj_t *tbl_users;
    lv_obj_t *btn_delete;
} objects_t;

static objects_t list_objects;

static void event_handler_cb_scr_list_btn_return_3(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_RELEASED) {
        lv_obj_clean(lv_scr_act());
        menu_screen();
    }
}

static char selected_data[32] = {0}; // Variável para armazenar o dado selecionado

static void table_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *table = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint32_t row, col;
        lv_table_get_selected_cell(table, &row, &col);

        if (row != LV_TABLE_CELL_NONE && col != LV_TABLE_CELL_NONE) { // Verifica se os índices são válidos
            const char *cell_value = lv_table_get_cell_value(table, row, col+1);
            if (cell_value) {
                snprintf(selected_data, sizeof(selected_data), "%s", cell_value);
                printf("Elemento selecionado: %s\n", selected_data);
            } else {
                printf("Erro: Valor da célula é NULL\n");
            }
        } else {
            printf("Erro: Índices de célula inválidos\n");
        }
    }
}

static void delete_confirm_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        const char *btn_text = lv_label_get_text(lv_obj_get_child(btn, 0)); // Obtém o texto do botão
        if (btn_text) {
            if (strcmp(btn_text, "Sim") == 0) {
                printf("Elemento deletado: %s\n", selected_data);
                // Aqui você pode adicionar a lógica para deletar o elemento
                char send_buf[256];
                snprintf(send_buf, sizeof(send_buf), "{\"cmd\":\"delete_user\",\"id\":\"%s\"}", selected_data);
                lvgl_socket_send(send_buf);
                printf("Sent: %s\n", send_buf);
            } else if (strcmp(btn_text, "Não") == 0) {
                printf("Operação de exclusão cancelada.\n");
            }
        }
        lv_obj_del(lv_event_get_user_data(e)); // Fecha a mensagem
        lv_obj_clean(lv_scr_act());
        delete_screen();
    }
}

static void event_handler_register_btn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("Delete button clicked\n");
        printf("Dado que será deletado: %s\n", selected_data);

        // Cria um fundo transparente levemente cinza
        lv_obj_t *overlay = lv_obj_create(list_objects.scr_list);
        lv_obj_set_pos(overlay, 0, 0);
        lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(overlay, lv_color_hex(UI_COLOR_TEXT_BLACK), 0);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
        lv_obj_set_style_border_width(overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

        // Cria a mensagem de confirmação manualmente
        lv_obj_t *msgbox = lv_obj_create(overlay);
        lv_obj_set_pos(msgbox, 0, 0);
        lv_obj_set_size(msgbox, 250, 150);
        lv_obj_clear_flag(msgbox, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align_to(msgbox, list_objects.scr_list, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t *label = lv_label_create(msgbox);
        lv_label_set_text(label, "Deseja deletar o usuario?");
        lv_obj_set_pos(label, 0, 0);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *btn_yes = lv_btn_create(msgbox);
        lv_obj_set_size(btn_yes, 80, 40);
        lv_obj_align(btn_yes, LV_ALIGN_BOTTOM_LEFT, 2, -10);
        lv_obj_add_event_cb(btn_yes, delete_confirm_event_cb, LV_EVENT_CLICKED, msgbox);
        lv_obj_set_style_bg_color(btn_yes, lv_color_hex(UI_COLOR_ACCENT_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(btn_yes, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        {
            lv_obj_t *label_yes = lv_label_create(btn_yes);
            lv_label_set_text(label_yes, "Sim");
            lv_obj_set_pos(label_yes, 0, 0);
            lv_obj_set_size(label_yes, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(label_yes, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(label_yes, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(label_yes, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        }


        lv_obj_t *btn_no = lv_btn_create(msgbox);
        lv_obj_set_size(btn_no, 80, 40);
        lv_obj_align(btn_no, LV_ALIGN_BOTTOM_RIGHT, -2, -10);
        lv_obj_add_event_cb(btn_no, delete_confirm_event_cb, LV_EVENT_CLICKED, msgbox);
        lv_obj_set_style_bg_color(btn_no, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(btn_no, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn_no, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        {
            lv_obj_t *label_no = lv_label_create(btn_no);
            lv_label_set_text(label_no, "Nao");
            lv_obj_set_pos(label_no, 0, 0);
            lv_obj_set_size(label_no, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(label_no, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(label_no, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(label_no, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        }
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
                label_draw_dsc->align = LV_ALIGN_TOP_LEFT;
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

const char* get_json_value(const char *json, jsmntok_t *tokens, int token_count, const char *key) {
    for (int i = 0; i < token_count; i++) {
        if (jsoneq(json, &tokens[i], key) == 0) {
            return json + tokens[i + 1].start; // Retorna o início do valor
        }
    }
    return NULL; // Retorna NULL se a chave não for encontrada
}

void delete_screen(void){
    static jsmn_parser p;
    static jsmntok_t t[128]; /* We expect no more than 128 tokens*/
    jsmn_init(&p);

    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    list_objects.scr_list = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {

            // tbl_users
            lv_obj_t *obj = lv_table_create(parent_obj);
            list_objects.tbl_users = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(50), LV_PCT(100));
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, table_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);

            /*Fill the first column*/
            lv_table_set_cell_value(obj, 0, 0, "Usuario");
            lv_table_set_cell_value(obj, 0, 1, "ID");


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
                            const char *id = get_json_value(recv_buf, t, r, "id");
                            char id_buf[32] = {0};
                            char usuario_buf[32] = {0};

                            if (usuario) {
                                snprintf(usuario_buf, sizeof(usuario_buf), "%.*s", t[i + 4].end - t[i + 4].start, recv_buf + t[i + 4].start);
                                lv_table_set_cell_value(obj, row, 0, usuario_buf);
                            }

                            if (id) {
                                snprintf(usuario_buf, sizeof(usuario_buf), "%.*s", t[i + 2].end - t[i + 2].start, recv_buf + t[i + 2].start);
                                lv_table_set_cell_value(obj, row, 1, usuario_buf);
                            }
                            printf("Usuario: %s\n", usuario_buf);

                            row++; // Incrementa a linha para o próximo objeto
                        }
                    }
                    lv_table_set_col_width(obj, 1, 0); // Define a largura da coluna como zero
                } else {
                    printf("Erro ao analisar o JSON ou chaves esperadas não encontradas.\n");
                }
            }
            /*Set a smaller height to the table. It'll make it scrollable*/
            lv_obj_set_height(obj, 300);
            /*Add an event callback to to apply some custom drawing*/
            lv_obj_add_event_cb(obj, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
        }
        {
            // Botão "Deletar"
            lv_obj_t *btn_register = lv_button_create(parent_obj);
            list_objects.btn_delete = btn_register;
            lv_obj_set_size(btn_register, 117, 50);
            lv_obj_add_event_cb(btn_register, event_handler_register_btn, LV_EVENT_CLICKED, NULL);
            // Posiciona o botão à direita e ao centro da tabela
            // if(list_objects.tbl_users) {
            //     lv_obj_align_to(btn_register, list_objects.tbl_users, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
            // } else {
            //     lv_obj_align(btn_register, LV_ALIGN_BOTTOM_MID, 0, 18);
            // }
            lv_obj_align_to(btn_register, list_objects.scr_list, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_obj_set_style_text_font(btn_register, &lv_font_montserrat_18, 0);
            lv_obj_set_style_bg_color(btn_register, lv_color_hex(UI_COLOR_ACCENT_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(btn_register, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(btn_register, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

            // Texto do botão
            lv_obj_t *btn_label = lv_label_create(btn_register);
            lv_label_set_text(btn_label, "Deletar");
            lv_obj_set_style_text_color(btn_label, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(btn_label);
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            list_objects.obj7 = obj;
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
                    list_objects.obj8 = obj;
                    lv_obj_set_pos(obj, 0, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Deletar");
                }
                {
                    // btn_return_3
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    list_objects.btn_return_3 = obj;
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
}
