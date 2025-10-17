#include "screens.h"

// --- Estruturas e Variáveis Globais --- //
typedef struct {
    lv_obj_t *scr_main;
    lv_obj_t *btn_register;
    lv_obj_t *header_bar;      // Renomeado de obj0
    lv_obj_t *header_title;    // Renomeado de obj1
    lv_obj_t *label_date;
    lv_obj_t *label_time;
    lv_timer_t *timer_clock;
} main_screen_objects_t;

static main_screen_objects_t g_main_screen; // Variável global para o estado da tela
static int g_last_day = -1;                 // Para otimizar atualização da data

// --- Protótipos das Funções --- //
static void event_handler_register_btn(lv_event_t *e);
static void timer_update_clock(lv_timer_t *timer);
static void update_label_date(lv_obj_t *label);
static void update_label_time(lv_obj_t *label);
static lv_obj_t* create_header_bar(lv_obj_t *parent);
static lv_obj_t* create_content_container(lv_obj_t *parent);
static void initialize_clock_labels(lv_obj_t *parent);

// --- Função Principal: Cria e Configura a Tela --- //
void main_screen(void) {
    // Limpa a tela ativa antes de criar a nova
    lv_obj_clean(lv_scr_act());

    // Cria o objeto raiz da tela
    g_last_day = -1;
    lv_obj_t *screen = lv_obj_create(lv_scr_act());
    g_main_screen.scr_main = screen;
    lv_obj_set_size(screen, 320, 480);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // Constrói a UI
    create_header_bar(screen);
    initialize_clock_labels(screen);

    // Cria o timer para atualização do relógio
    g_main_screen.timer_clock = lv_timer_create(timer_update_clock, 1000, &g_main_screen);
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
    lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 0);
    update_label_date(label_date); // Inicializa com a data atual

    // Label da Hora
    lv_obj_t *label_time = lv_label_create(container);
    g_main_screen.label_time = label_time;
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_44, 0);
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
    update_label_time(label_time); // Inicializa com a hora atual

    // Botão "Cadastrar"
    lv_obj_t *btn_register = lv_button_create(container);
    g_main_screen.btn_register = btn_register;
    lv_obj_set_size(btn_register, 117, 50);
    lv_obj_add_event_cb(btn_register, event_handler_register_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn_register, LV_ALIGN_BOTTOM_MID, 0, 18);
    lv_obj_set_style_text_font(btn_register, &lv_font_montserrat_18, 0);
    lv_obj_set_style_bg_color(btn_register, lv_color_hex(0x00DE690C), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_register, lv_color_hex(0x00000000), LV_PART_ITEMS | LV_STATE_DEFAULT);

    // Texto do botão
    lv_obj_t *btn_label = lv_label_create(btn_register);
    lv_label_set_text(btn_label, "Menu");
    lv_obj_center(btn_label);
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

// --- Cria a Barra de Cabeçalho Superior --- //
static lv_obj_t* create_header_bar(lv_obj_t *parent) {
    lv_obj_t *bar = lv_obj_create(parent);
    g_main_screen.header_bar = bar;
    lv_obj_set_size(bar, LV_PCT(100), LV_PCT(10));
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x005C5C5C), 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0x000D0D0D), 0);
    lv_obj_set_style_border_width(bar, 1, 0);

    // Título do cabeçalho
    lv_obj_t *title = lv_label_create(bar);
    g_main_screen.header_title = title;
    lv_label_set_text(title, "Home");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00000000), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 1);

    return bar;
}

// --- Handler do Botão "Cadastrar" --- //
static void event_handler_register_btn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // Pausa o timer antes de mudar de tela
        if (g_main_screen.timer_clock) {
            lv_timer_pause(g_main_screen.timer_clock);
        }
        // Carrega a próxima tela
        menu_screen();
    }
}