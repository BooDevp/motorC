#ifndef MENU_LATERAL_H
#define MENU_LATERAL_H

#include "ui/botones.h"
#include "core/tipos.h"

typedef struct
{
    int n_botones;
    Boton *botones;

    FunctionCargarEscena params_escena;

    Uint64 ultimo_clic;
    Uint64 COOLDOWN_BOTON;

} MenuLateral;

void menuLateral_inicializar(MenuLateral *menu, Mundo mundo, Arena *arena_ui);
void menuLateral_renderizar(MenuLateral *menu, SDL_Renderer *renderer);

#endif