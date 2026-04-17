#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "modelo.h"
#include "gestion_memoria.h"
#include "render.h"

// Variables internas
static bool g_hover_any_btn = false;
static SDL_Cursor *cursor_mano = NULL;
static SDL_Cursor *cursor_normal = NULL;

typedef struct
{
    float x, y, w, h;
    SDL_Color color;    
    const char *etiqueta;
    void (*accion)(void *params);
    void *params;
} Boton;

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
} FunctionCargarModelo;

// Acciones de los botones
static bool is_mouse_hover(float mx, float my, Boton b) {
    return (mx >= b.x && mx <= b.x + b.w &&
            my >= b.y && my <= b.y + b.h);
}

void ui_comenzar_frame() {
    g_hover_any_btn = false;
}

static void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params)
{
    float mx, my;
    uint32_t estado_raton = SDL_GetMouseState(&mx, &my);
    bool hover = is_mouse_hover(mx, my, *b);

    if (hover) {
        b->color = (SDL_Color){0, 200, 0, 255};
        g_hover_any_btn = true;

        if (estado_raton & SDL_BUTTON_LMASK) {
            if (b->accion != NULL) b->accion(params);
        }
    } else {
        b->color = (SDL_Color){100, 100, 100, 255};
    }

    // Dibujo del botón
    SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b, 255);
    SDL_FRect rect_btn = {b->x, b->y, b->w, b->h};

    // Dibujo el texto del boton
    SDL_RenderFillRect(renderer, &rect_btn);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, b->x + 10, b->y + 12, b->etiqueta);
}

void gestionar_cursor_raton() {
    if (!cursor_mano || !cursor_normal) return;

    if (g_hover_any_btn) {
        SDL_SetCursor(cursor_mano);
    } else {
        SDL_SetCursor(cursor_normal);
    }
}

void ui_init() {
    if (!cursor_mano) cursor_mano = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    if (!cursor_normal) cursor_normal = SDL_GetDefaultCursor();
}


// Acciones de los botones
void accion_cambiar_vsync(void *datos)
{
    FunctionCambioVsync *d = (FunctionCambioVsync *)datos;
    cambiar_vsync(d->renderer, d->bool_vsync, d->ultimo_clic, d->COOLDOWN_BOTON);
}

void accion_cargar_modelo(void *datos)
{
    FunctionCargarModelo *d = (FunctionCargarModelo *)datos;
    cargar_modelo(d->modelo, d->ultimo_clic, d->COOLDOWN_BOTON, d->arena);
}

#endif