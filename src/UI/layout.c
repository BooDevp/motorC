#define SDL_MAIN_HANDLED
#include "ui/layout.h"
#include "ui/menuLateral.h"
#include "core/tipos.h"

Uint64 frames_contados = 0;
float fps_actuales = 0, dt = 0;
char texto_fps[64] = "Iniciando...";

void layout_inicializar(Layout *layout, Arena *arena_ui, SDL_Renderer *renderer, Mundo mundo, VentanaInfo ventana_info, MenuLateral *menu)
{
    cursores_init();

    layout->barra_inferior_h = (int)(ventana_info.alto * UI_BARRA_RATIO);
    layout->menu_lateral_w = (int)(ventana_info.ancho * UI_MENU_RATIO);
    layout->area_util_h = ventana_info.alto - layout->barra_inferior_h;
    layout->juego_w = ventana_info.ancho - layout->menu_lateral_w;
    layout->juego_h = layout->area_util_h;
    layout->juego_offset_x = layout->menu_lateral_w;
    layout->juego_offset_y = 0;
    layout->escala_juego = (layout->juego_w / 2.0f) * ventana_info.zoom;

    layout->ventana_alto = ventana_info.alto;
    layout->ventana_ancho = ventana_info.ancho;
    layout->tiempo_anterior_fps = ventana_info.tiempo_fps;

    menuLateral_inicializar(menu, mundo, arena_ui);

    arena_reporte(arena_ui, "DESPUES DE INICIALIZAR LAYOUT/UI");
}

void layout_actualizar(Layout *layout)
{
    ui_comenzar_frame();
}

void layout_pintar_marcos(Layout *layout, SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.marco.r, TEMA_DEFAULT.marco.g, TEMA_DEFAULT.marco.b, TEMA_DEFAULT.marco.a);
    SDL_RenderLine(renderer, 0, layout->area_util_h, layout->ventana_ancho, layout->area_util_h);
    SDL_RenderLine(renderer, layout->menu_lateral_w, 0, layout->menu_lateral_w, layout->area_util_h);
}

void layout_pintar_textos(Layout *layout, SDL_Renderer *renderer)
{
    calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), layout->tiempo_anterior_fps);
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, TEMA_DEFAULT.modelo.a);
    SDL_RenderDebugText(renderer, 10, 10, texto_fps);

    // En ui_pintar_textos
    char info_status[128];
    const char *cursor = (SDL_GetTicks() / 500 % 2 == 0) ? ">" : " "; // Parpadeo
    SDL_snprintf(info_status, sizeof(info_status), "OS_CORE: ACTIVE | SYSTEM_VAL: %.2f EUR %s", 50000.0f, cursor);
    SDL_RenderDebugText(renderer, 20, layout->ventana_alto - (layout->barra_inferior_h / 2) - 4, info_status);
}

void layout_renderizar(Layout *layout, SDL_Renderer *renderer)
{
    layout_pintar_marcos(layout, renderer);
    layout_pintar_textos(layout, renderer);
}
