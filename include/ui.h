#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "modelo.h"
#include "gestion_memoria.h"

typedef struct
{
    float x, y, w, h;
    SDL_Color color;    
    const char *etiqueta;
    void (*accion)(void *params);
    void *params;
} Boton;

void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params);
void ui_init();
void ui_comenzar_frame();
void gestionar_cursor_raton();

// Struct botones
typedef struct
{
    SDL_Renderer *renderer;
    bool *bool_vsync;
    Uint64 *ultimo_clic;
    Uint64 COOLDOWN_BOTON;
} FunctionCambioVsync;

typedef struct
{
    Modelo **modelo;
    Uint64 *ultimo_clic;
    const Uint64 COOLDOWN_BOTON;
    Arena *arena;
    SDL_GPUDevice *gpu;
} FunctionCargarModelo;

// Acciones de los botones
void accion_cambiar_vsync(void *datos);
void accion_cargar_modelo(void *datos);

#endif