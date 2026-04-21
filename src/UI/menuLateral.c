#define SDL_MAIN_HANDLED
#include "ui/menuLateral.h"
#include "core/tipos.h"

void menuLateral_inicializar(MenuLateral *menu,  Mundo mundo, Arena *arena_ui)
{
    menu->COOLDOWN_BOTON = 200;
    menu->ultimo_clic = 0;

    menu->params_escena.escena = mundo.escena_actual;
    menu->params_escena.ultimo_clic = &menu->ultimo_clic;
    menu->params_escena.COOLDOWN_BOTON = menu->COOLDOWN_BOTON;
    menu->params_escena.escena_id = 1;
    menu->params_escena.arena = mundo.arena_escena;
    menu->params_escena.modelos_globales = mundo.modelos_globales;
    menu->params_escena.n_modelos_globales = TOTAL_MODELOS;

    menu->n_botones = 1;
    menu->botones = (Boton *)arena_push(arena_ui, sizeof(Boton) * menu->n_botones);
    menu->botones[0] = (Boton){20, 50, 140, 30, {100, 100, 100, 255}, "CARGAR ESCENA 1", accion_cargar_escena, &menu->params_escena};
}

void menuLateral_renderizar(MenuLateral *menu, SDL_Renderer *renderer)
{    
    for (int i = 0; i < menu->n_botones; i++)
    {
        ui_dibujar_botones_menu_lateral(renderer, &menu->botones[i], menu->botones[i].params);
    }
    gestionar_cursor_raton();
}
