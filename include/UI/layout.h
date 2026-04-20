#ifndef LAYOUT_H
#define LAYOUT_H

#include <SDL3/SDL.h>
#include <stdlib.h>
#include "UI/botones.h"
#include "style.h"
#include "render.h"

// Layout defines
#define UI_MENU_RATIO 0.25f  // 25% de ancho para el menú lateral
#define UI_BARRA_RATIO 0.05f // 10% de alto para la barra inferior

// Global variables for layout
extern Uint64 frames_contados;
extern float fps_actuales;
extern float dt;
extern char texto_fps[64];

typedef struct
{
    Boton *botones;
    int n_botones;
    Uint64 ultimo_clic;
    Uint64 COOLDOWN_BOTON;
    bool bool_vsync;
    FunctionCambioVsync params_vsync;
    FunctionCargarEscena params_escena;
    
    int barra_inferior_h;
    int menu_lateral_w;
    int area_util_h;
    int juego_w;
    int juego_h;
    int juego_offset_x;
    int juego_offset_y;
    float escala_juego;

    int ventana_ancho;
    int ventana_alto;

    Uint64 *tiempo_anterior_fps;
} UI;

// Function declarations
void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Escena **escena_actual, Arena *arena_escena, Modelo **modelos_globales, int ventana_ancho, int ventana_alto, float zoom, Uint64 *tiempo_anterior_fps);
void ui_actualizar(UI *ui);
void ui_pintar_marcos(UI *ui, SDL_Renderer *renderer);
void ui_pintar_textos(UI *ui, SDL_Renderer *renderer);
void ui_renderizar(UI *ui, SDL_Renderer *renderer);

#endif