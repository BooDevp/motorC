#define SDL_MAIN_HANDLED
#include "ui/layout.h"
#include "core/gestion_memoria.h"

Uint64 frames_contados = 0;
float fps_actuales = 0, dt = 0;
char texto_fps[64] = "Iniciando...";

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Escena **escena_actual, Arena *arena_escena, Modelo **modelos_globales, int ventana_ancho, int ventana_alto, float zoom, Uint64 *tiempo_anterior_fps)
{
    ui_init();
    
    ui->barra_inferior_h = (int)(ventana_alto * UI_BARRA_RATIO);
    ui->menu_lateral_w = (int)(ventana_ancho * UI_MENU_RATIO);
    ui->area_util_h = ventana_alto - ui->barra_inferior_h;
    ui->juego_w = ventana_ancho - ui->menu_lateral_w;
    ui->juego_h = ui->area_util_h;
    ui->juego_offset_x = ui->menu_lateral_w;
    ui->juego_offset_y = 0;
    ui->escala_juego = (ui->juego_w / 2.0f) * zoom;

    ui->ventana_alto = ventana_alto;
    ui->ventana_ancho = ventana_ancho;
    ui->tiempo_anterior_fps = tiempo_anterior_fps;

    ui->COOLDOWN_BOTON = 200;
    ui->ultimo_clic = 0;
    ui->bool_vsync = true;

    ui->params_vsync.renderer = renderer;
    ui->params_vsync.bool_vsync = &ui->bool_vsync;
    ui->params_vsync.ultimo_clic = &ui->ultimo_clic;
    ui->params_vsync.COOLDOWN_BOTON = ui->COOLDOWN_BOTON;

    ui->params_escena.escena = escena_actual;
    ui->params_escena.ultimo_clic = &ui->ultimo_clic;
    ui->params_escena.COOLDOWN_BOTON = ui->COOLDOWN_BOTON;
    ui->params_escena.escena_id = 1;
    ui->params_escena.arena = arena_escena;
    ui->params_escena.modelos_globales = modelos_globales;
    ui->params_escena.n_modelos_globales = TOTAL_MODELOS;

    ui->n_botones = 2;
    ui->botones = (Boton *)arena_push(arena_ui, sizeof(Boton) * ui->n_botones);
    ui->botones[0] = (Boton){20, 50, 140, 30, {100, 100, 100, 255}, "VSYNC ON/OFF", accion_cambiar_vsync, &ui->params_vsync};
    ui->botones[1] = (Boton){20, 90, 140, 30, {100, 100, 100, 255}, "CARGAR ESCENA 1", accion_cargar_escena, &ui->params_escena};

    arena_reporte(arena_ui, "DESPUES DE INICIALIZAR UI");
}

void ui_actualizar(UI *ui)
{
    ui_comenzar_frame();
}

void ui_pintar_marcos(UI *ui, SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.marco.r, TEMA_DEFAULT.marco.g, TEMA_DEFAULT.marco.b, TEMA_DEFAULT.marco.a);
    SDL_RenderLine(renderer, 0, ui->area_util_h, ui->ventana_ancho, ui->area_util_h);
    SDL_RenderLine(renderer, ui->menu_lateral_w, 0, ui->menu_lateral_w, ui->area_util_h);
}

void ui_pintar_textos(UI *ui, SDL_Renderer *renderer)
{
    calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), ui->tiempo_anterior_fps);
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, TEMA_DEFAULT.modelo.a);
    SDL_RenderDebugText(renderer, 10, 10, texto_fps);

    // En ui_pintar_textos
    char info_status[128];
    const char *cursor = (SDL_GetTicks() / 500 % 2 == 0) ? ">" : " "; // Parpadeo
    SDL_snprintf(info_status, sizeof(info_status), "OS_CORE: ACTIVE | SYSTEM_VAL: %.2f EUR %s", 50000.0f, cursor);
    SDL_RenderDebugText(renderer, 20, ui->ventana_alto - (ui->barra_inferior_h / 2) - 4, info_status);
}

void ui_renderizar(UI *ui, SDL_Renderer *renderer)
{
    ui_pintar_marcos(ui, renderer);
    ui_pintar_textos(ui, renderer);
    for (int i = 0; i < ui->n_botones; i++)
    {
        ui_dibujar_botones_menu_lateral(renderer, &ui->botones[i], ui->botones[i].params);
    }
    gestionar_cursor_raton();
}
