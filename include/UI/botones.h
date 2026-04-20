#ifndef BOTONES_H
#define BOTONES_H

#include <SDL3/SDL.h>
#include <stdlib.h>
#include "scene.h"
#include "modelo.h"
#include "gestion_memoria.h"
#include "render.h"
#include "scenes/scene1.h"
#include "modelos_id.h"

// Structs
typedef struct
{
    float x, y, w, h;
    SDL_Color color;
    const char *etiqueta;
    void (*accion)(void *params);
    void *params;
} Boton;

typedef struct
{
    SDL_Renderer *renderer;
    bool *bool_vsync;
    Uint64 *ultimo_clic;
    Uint64 COOLDOWN_BOTON;
} FunctionCambioVsync;

typedef struct
{
    Escena **escena;
    Uint64 *ultimo_clic;
    Uint64 COOLDOWN_BOTON;
    int escena_id;
    Arena *arena;
    Modelo **modelos_globales;
    int n_modelos_globales;
} FunctionCargarEscena;

// Function declarations
bool is_mouse_hover(float mx, float my, Boton b);
void ui_comenzar_frame();
void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params);
void gestionar_cursor_raton();
void ui_init();
void accion_cambiar_vsync(void *datos);
void accion_cargar_escena(void *datos);

#endif