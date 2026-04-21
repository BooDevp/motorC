#ifndef BOTONES_H
#define BOTONES_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "graphics/scene.h"
#include "graphics/scenes/scene1.h"
#include "config/modelos_id.h"

typedef struct {
    Escena **escena;
    Uint64 *ultimo_clic;
    Uint64 COOLDOWN_BOTON;
    int escena_id;
    Arena *arena;
    Modelo **modelos_globales;
    int n_modelos_globales;
} FunctionCargarEscena;

typedef struct {
    float x, y, w, h;
    SDL_Color color;
    const char *etiqueta;
    void (*accion)(void *);
    void *params;
} Boton;

bool is_mouse_hover(float mx, float my, Boton b);
void ui_comenzar_frame();
void ui_dibujar_botones_menu_lateral(SDL_Renderer *renderer, Boton *b, void *params);
void gestionar_cursor_raton();
void cursores_init();
void accion_cargar_escena(void *datos);

#endif