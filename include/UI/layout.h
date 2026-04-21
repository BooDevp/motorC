#ifndef LAYOUT_H
#define LAYOUT_H

#include <SDL3/SDL.h>
#include "ui/botones.h"
#include "config/style.h"
#include "graphics/scene.h"
#include "core/utils.h"
#include "ui/menuLateral.h"
#include "core/tipos.h"

#define UI_MENU_RATIO 0.25f
#define UI_BARRA_RATIO 0.15f

typedef struct
{
    int ventana_ancho, ventana_alto;
    int menu_lateral_w;
    int barra_inferior_h;
    int area_util_h;
    int juego_w, juego_h;
    int juego_offset_x, juego_offset_y;
    float escala_juego;    

    Uint64 *tiempo_anterior_fps;    
} UI;

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Mundo mundo, VentanaInfo ventana_info, MenuLateral *menu);
void ui_actualizar(UI *ui);
void ui_pintar_marcos(UI *ui, SDL_Renderer *renderer);
void ui_pintar_textos(UI *ui, SDL_Renderer *renderer);
void ui_renderizar(UI *ui, SDL_Renderer *renderer);

#endif