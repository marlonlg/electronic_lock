#include "screens.h"

#define MAIN_SCREEN_POLL_INTERVAL_MS 500
#define MAIN_SCREEN_RECV_TIMEOUT_MS 50
#define MAIN_SCREEN_AUTH_MODAL_TIMEOUT_MS 2000
#define MAIN_SCREEN_POLL_CMD "{\"cmd\":\"read_main_tag\"}"
#define MAIN_SCREEN_MAX_JSON_TOKENS 32
#define MAIN_SCREEN_MAX_JSON_VALUE 256

// --- Estruturas e Variáveis Globais --- //
typedef struct {
    lv_obj_t *scr_main;
    lv_obj_t *btn_register;
    lv_obj_t *label_date;
    lv_obj_t *label_time;
    lv_timer_t *timer_clock;
    lv_timer_t *timer_poll;
    lv_obj_t *auth_modal;
} main_screen_objects_t;

static main_screen_objects_t g_main_screen; // Variável global para o estado da tela
static int g_last_day = -1;                 // Para otimizar atualização da data

// --- Protótipos das Funções --- //
static void event_handler_register_btn(lv_event_t *e);
static void timer_update_clock(lv_timer_t *timer);
static void timer_poll_tag(lv_timer_t *timer);
static void update_label_date(lv_obj_t *label);
static void update_label_time(lv_obj_t *label);
static lv_obj_t* create_content_container(lv_obj_t *parent);
static void initialize_clock_labels(lv_obj_t *parent);
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
static bool json_get_string(const char *json, jsmntok_t *tokens, int token_count, const char *key, char *out, size_t out_size);
static void normalize_image_path(const char *src, char *dst, size_t dst_size);
static void show_authorized_modal(const char *usuario, const char *img_path);
static void close_authorized_modal(lv_event_t *e);
static void timer_close_authorized_modal(lv_timer_t *timer);

// --- Função Principal: Cria e Configura a Tela --- //
void main_screen(void) {
    // Limpa a tela ativa antes de criar a nova
    lv_obj_clean(lv_scr_act());
    if (g_main_screen.timer_clock) {
        lv_timer_del(g_main_screen.timer_clock);
        g_main_screen.timer_clock = NULL;
    }
    if (g_main_screen.timer_poll) {
        lv_timer_del(g_main_screen.timer_poll);
        g_main_screen.timer_poll = NULL;
    }
    g_main_screen.auth_modal = NULL;

    // Cria o objeto raiz da tela
    g_last_day = -1;
    lv_obj_t *screen = lv_obj_create(lv_scr_act());
    g_main_screen.scr_main = screen;
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_SCREEN_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(screen, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Constrói a UI
    create_header(screen, "Tela Principal", NULL);
    initialize_clock_labels(screen);

    // Cria o timer para atualização do relógio
    g_main_screen.timer_clock = lv_timer_create(timer_update_clock, 1000, &g_main_screen);
    g_main_screen.timer_poll = lv_timer_create(timer_poll_tag, MAIN_SCREEN_POLL_INTERVAL_MS, &g_main_screen);
}

// --- Função de Atualização do Timer --- //
static void timer_update_clock(lv_timer_t *timer) {
    main_screen_objects_t *screen_data = lv_timer_get_user_data(timer);
    if (!screen_data) return;

    if (screen_data->label_time) {
        update_label_time(screen_data->label_time);
    }
    if (screen_data->label_date) {
        update_label_date(screen_data->label_date);
    }
}

static void timer_poll_tag(lv_timer_t *timer) {
    main_screen_objects_t *screen_data = lv_timer_get_user_data(timer);
    char recv_buf[512];
    jsmn_parser parser;
    jsmntok_t tokens[MAIN_SCREEN_MAX_JSON_TOKENS];
    char cmd[MAIN_SCREEN_MAX_JSON_VALUE];
    char usuario[MAIN_SCREEN_MAX_JSON_VALUE];
    char img_path[MAIN_SCREEN_MAX_JSON_VALUE];

    if (!screen_data || screen_data->auth_modal) return;

    if (lvgl_socket_send(MAIN_SCREEN_POLL_CMD) < 0) return;
    if (lvgl_socket_recv_timeout(recv_buf, sizeof(recv_buf), MAIN_SCREEN_RECV_TIMEOUT_MS) < 0) return;

    jsmn_init(&parser);
    int token_count = jsmn_parse(&parser, recv_buf, strlen(recv_buf), tokens, sizeof(tokens) / sizeof(tokens[0]));
    if (token_count <= 0) return;

    if (!json_get_string(recv_buf, tokens, token_count, "cmd", cmd, sizeof(cmd))) return;
    if (strcmp(cmd, "authorized") != 0) return;
    if (!json_get_string(recv_buf, tokens, token_count, "usuario", usuario, sizeof(usuario))) return;
    if (!json_get_string(recv_buf, tokens, token_count, "img_path", img_path, sizeof(img_path))) return;

    show_authorized_modal(usuario, img_path);
}

// --- Funções de Atualização dos Labels --- //
static void update_label_time(lv_obj_t *label) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);

    lv_label_set_text_fmt(label, "%02d:%02d:%02d",
        time_info->tm_hour,
        time_info->tm_min,
        time_info->tm_sec);
}

static void update_label_date(lv_obj_t *label) {
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    int current_day = time_info->tm_mday;

    // Atualiza apenas se o dia mudou (otimização)
    if (current_day != g_last_day) {
        lv_label_set_text_fmt(label, "%02d/%02d/%04d",
            time_info->tm_mday,
            time_info->tm_mon + 1,
            time_info->tm_year + 1900);
        g_last_day = current_day;
    }
}

// --- Função de Inicialização dos Labels de Relógio --- //
static void initialize_clock_labels(lv_obj_t *parent) {
    lv_obj_t *container = create_content_container(parent);

    // Label da Data
    lv_obj_t *label_date = lv_label_create(container);
    g_main_screen.label_date = label_date;
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_44, 0);
    lv_obj_set_style_text_color(label_date, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 0);
    update_label_date(label_date); // Inicializa com a data atual

    // Label da Hora
    lv_obj_t *label_time = lv_label_create(container);
    g_main_screen.label_time = label_time;
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_44, 0);
    lv_obj_set_style_text_color(label_time, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
    update_label_time(label_time); // Inicializa com a hora atual

    // Botão "Cadastrar"
    lv_obj_t *btn_register = lv_button_create(container);
    g_main_screen.btn_register = btn_register;
    lv_obj_set_size(btn_register, 117, 50);
    lv_obj_add_event_cb(btn_register, event_handler_register_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn_register, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(btn_register, &lv_font_montserrat_18, 0);
    lv_obj_set_style_bg_color(btn_register, lv_color_hex(UI_COLOR_ACCENT_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_register, lv_color_hex(UI_COLOR_TEXT_BLACK), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_register, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Texto do botão
    lv_obj_t *btn_label = lv_label_create(btn_register);
    lv_label_set_text(btn_label, "Menu");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(UI_COLOR_TEXT_WHITE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn_label);
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

static void normalize_image_path(const char *src, char *dst, size_t dst_size) {
    if (!src || !dst || dst_size == 0) return;

    if (src[0] == '/' && dst_size > 3) {
        lv_snprintf(dst, dst_size, "A:%s", src);
        return;
    }

    lv_snprintf(dst, dst_size, "%s", src);
}

static void close_authorized_modal(lv_event_t *e) {
    lv_obj_t *modal = lv_event_get_user_data(e);

    if (modal) {
        lv_obj_delete(modal);
    }

    g_main_screen.auth_modal = NULL;
    if (g_main_screen.timer_poll) {
        lv_timer_resume(g_main_screen.timer_poll);
    }
}

static void timer_close_authorized_modal(lv_timer_t *timer) {
    lv_obj_t *modal = lv_timer_get_user_data(timer);

    if (modal) {
        lv_obj_delete(modal);
    }

    g_main_screen.auth_modal = NULL;
    if (g_main_screen.timer_poll) {
        lv_timer_resume(g_main_screen.timer_poll);
    }

    lv_timer_del(timer);
}

static void show_authorized_modal(const char *usuario, const char *img_path) {
    char normalized_path[MAIN_SCREEN_MAX_JSON_VALUE];
    lv_obj_t *modal;
    lv_obj_t *panel;
    lv_obj_t *title;
    lv_obj_t *image;
    lv_obj_t *close_btn;
    lv_obj_t *close_label;

    normalize_image_path(img_path, normalized_path, sizeof(normalized_path));

    modal = lv_obj_create(g_main_screen.scr_main);
    g_main_screen.auth_modal = modal;
    lv_obj_set_size(modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(modal, LV_OPA_80, 0);
    lv_obj_set_style_border_width(modal, 0, 0);
    lv_obj_clear_flag(modal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(modal);

    panel = lv_obj_create(modal);
    lv_obj_set_size(panel, LV_PCT(78), LV_PCT(82));
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(UI_COLOR_HEADER_BG_DARK), 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 12, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    title = lv_label_create(panel);
    lv_label_set_text_fmt(title, "Acesso liberado\n%s", usuario);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_TEXT_WHITE), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    image = lv_image_create(panel);
    lv_obj_set_size(image, LV_PCT(100), LV_PCT(72));
    lv_obj_align(image, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_image_set_inner_align(image, LV_IMAGE_ALIGN_CONTAIN);
    lv_image_set_src(image, normalized_path);

    close_btn = lv_button_create(panel);
    lv_obj_set_size(close_btn, 42, 42);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(UI_COLOR_ACCENT_BLUE), 0);
    lv_obj_set_style_border_width(close_btn, 0, 0);
    lv_obj_add_event_cb(close_btn, close_authorized_modal, LV_EVENT_CLICKED, modal);

    close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "X");
    lv_obj_center(close_label);

    if (g_main_screen.timer_poll) {
        lv_timer_pause(g_main_screen.timer_poll);
    }

    lv_timer_t *close_timer = lv_timer_create(timer_close_authorized_modal, MAIN_SCREEN_AUTH_MODAL_TIMEOUT_MS, modal);
    if (close_timer) {
        lv_timer_set_repeat_count(close_timer, 1);
    }
}

// --- Cria o Container de Conteúdo Principal --- //
static lv_obj_t* create_content_container(lv_obj_t *parent) {
    lv_obj_t *container = lv_obj_create(parent);
    // lv_obj_set_size(container, 470, 270);
    lv_obj_set_size(container, 470, 270);
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, LV_PCT(13));
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0); // Fundo transparente
    lv_obj_set_style_border_width(container, 0, 0);

    return container;
}

// --- Handler do Botão "Cadastrar" --- //
static void event_handler_register_btn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Pausa o timer antes de mudar de tela
        if (g_main_screen.timer_clock) {
            lv_timer_pause(g_main_screen.timer_clock);
        }
        if (g_main_screen.timer_poll) {
            lv_timer_pause(g_main_screen.timer_poll);
        }
        // Carrega a próxima tela
        menu_screen();
    }
}
