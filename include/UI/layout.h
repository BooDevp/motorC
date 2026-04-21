#ifndef LAYOUT_H
#define LAYOUT_H

#include "ui/layout.h"
#include "ui/menuLateral.h"
#include "core/tipos.h"

#define UI_MENU_RATIO 0.25f
#define UI_BARRA_RATIO 0.15f

void layout_inicializar(Layout *layout, Arena *arena_ui, SDL_Renderer *renderer, Mundo mundo, VentanaInfo ventana_info, MenuLateral *menu);
void layout_actualizar(Layout *layout);
void layout_pintar_marcos(Layout *layout, SDL_Renderer *renderer);
void layout_pintar_textos(Layout *layout, SDL_Renderer *renderer);
void layout_renderizar(Layout *layout, SDL_Renderer *renderer);

#endif