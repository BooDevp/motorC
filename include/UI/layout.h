#ifndef LAYOUT_H
#define LAYOUT_H

#include <SDL3/SDL.h>
#include "ui/botones.h"
#include "config/style.h"
#include "graphics/scene.h"
#include "core/utils.h"

#define UI_MENU_RATIO 0.25f
#define UI_BARRA_RATIO 0.15f

typedef struct {
    int ventana_ancho, ventana_alto;
    int menu_lateral_w;
    int barra_inferior_h;
    int area_util_h;
    int juego_w, juego_h;
    int juego_offset_x, juego_offset_y;
    float escala_juego;

    int n_botones;
    Boton *botones;

    FunctionCambioVsync params_vsync;
    FunctionCargarEscena params_escena;

    bool bool_vsync;
    Uint64 ultimo_clic;
    Uint64 COOLDOWN_BOTON;

    Uint64 *tiempo_anterior_fps;
} UI;

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Escena **escena_actual, Arena *arena_escena, Modelo **modelos_globales, int ventana_ancho, int ventana_alto, float zoom, Uint64 *tiempo_anterior_fps);
void ui_actualizar(UI *ui);
void ui_pintar_marcos(UI *ui, SDL_Renderer *renderer);
void ui_pintar_textos(UI *ui, SDL_Renderer *renderer);
void ui_renderizar(UI *ui, SDL_Renderer *renderer);

#endif