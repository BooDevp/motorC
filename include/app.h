#ifndef APP_H
#define APP_H

#include "core/engine.h"
#include "core/gestor_memoria.h"
#include "graphics/layout.h"
#include "graphics/camara.h"
#include "graphics/escena.h"
#include "ui/menu_lateral.h"

typedef struct
{
    Engine motor;
    GestorMemoria memoria;
    Camara camara;
    Layout layout;
    MenuLateral menu;
    Escena *escena_actual;
    Modelo **catalogo;
    SDL_Cursor *cursor_mano;
    SDL_Cursor *cursor_flecha;
} App;

bool app_init(App *app);
void app_run(App *app);
void app_shutdown(App *app);

#endif