#ifndef MENU_LATERAL_H
#define MENU_LATERAL_H

#include "graphics/layout.h"
#include "ui/boton.h"

#define MAX_BOTONES_MENU 5

typedef struct {
    Boton botones[MAX_BOTONES_MENU];
    int contador_botones;
} MenuLateral;

void menu_lateral_init(MenuLateral *menu, Layout *layout);
void menu_lateral_actualizar(MenuLateral *menu, float mouse_x, float mouse_y);
void menu_lateral_dibujar(SDL_Renderer *renderer, MenuLateral *menu);
void menu_lateral_gestionar_clic(MenuLateral *menu, float mouse_x, float mouse_y);

#endif