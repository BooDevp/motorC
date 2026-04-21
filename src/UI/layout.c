#define SDL_MAIN_HANDLED
#include "ui/layout.h"
#include "core/gestion_memoria.h"
#include "ui/menuLateral.h"
#include "core/tipos.h"

Uint64 frames_contados = 0;
float fps_actuales = 0, dt = 0;
char texto_fps[64] = "Iniciando...";

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Mundo mundo, VentanaInfo ventana_info, MenuLateral *menu)
{
    cursores_init();

    ui->barra_inferior_h = (int)(ventana_info.alto * UI_BARRA_RATIO);
    ui->menu_lateral_w = (int)(ventana_info.ancho * UI_MENU_RATIO);
    ui->area_util_h = ventana_info.alto - ui->barra_inferior_h;
    ui->juego_w = ventana_info.ancho - ui->menu_lateral_w;
    ui->juego_h = ui->area_util_h;
    ui->juego_offset_x = ui->menu_lateral_w;
    ui->juego_offset_y = 0;
    ui->escala_juego = (ui->juego_w / 2.0f) * ventana_info.zoom;

    ui->ventana_alto = ventana_info.alto;
    ui->ventana_ancho = ventana_info.ancho;
    ui->tiempo_anterior_fps = ventana_info.tiempo_fps;

    menuLateral_inicializar(menu, mundo, arena_ui);

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
}
