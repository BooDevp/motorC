#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <stdlib.h>
#include "scene.h"
#include "modelo.h"
#include "gestion_memoria.h"
#include "render.h"

// Escenas
#include "scenes/scene1.h"

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
    Uint64 COOLDOWN_BOTON;
    Arena *arena;
} FunctionCargarModelo;

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

typedef struct
{
    Boton *botones;
    int n_botones;
    Uint64 ultimo_clic;
    Uint64 COOLDOWN_BOTON;
    bool bool_vsync;
    FunctionCambioVsync params_vsync;
    FunctionCargarEscena params_escena;
} UI;

// Acciones de los botones
static bool is_mouse_hover(float mx, float my, Boton b)
{
    return (mx >= b.x && mx <= b.x + b.w &&
            my >= b.y && my <= b.y + b.h);
}

void ui_comenzar_frame()
{
    g_hover_any_btn = false;
}

static void ui_dibujar_boton(SDL_Renderer *renderer, Boton *b, void *params)
{
    float mx, my;
    uint32_t estado_raton = SDL_GetMouseState(&mx, &my);
    bool hover = is_mouse_hover(mx, my, *b);

    if (hover)
    {
        b->color = (SDL_Color){0, 200, 0, 255};
        g_hover_any_btn = true;

        if (estado_raton & SDL_BUTTON_LMASK)
        {
            if (b->accion != NULL)
                b->accion(params);
        }
    }
    else
    {
        b->color = (SDL_Color){100, 100, 100, 255};
    }

    // Dibujo del botón
    SDL_SetRenderDrawColor(renderer, b->color.r, b->color.g, b->color.b, 255);
    SDL_FRect rect_btn = {b->x, b->y, b->w, b->h};

    // Dibujo el texto del boton
    SDL_RenderRect(renderer, &rect_btn);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, b->x + 10, b->y + 12, b->etiqueta);
}

void gestionar_cursor_raton()
{
    if (!cursor_mano || !cursor_normal)
        return;

    if (g_hover_any_btn)
    {
        SDL_SetCursor(cursor_mano);
    }
    else
    {
        SDL_SetCursor(cursor_normal);
    }
}

void ui_init()
{
    if (!cursor_mano)
        cursor_mano = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    if (!cursor_normal)
        cursor_normal = SDL_GetDefaultCursor();
}

// Acciones de los botones
void accion_cambiar_vsync(void *datos)
{
    FunctionCambioVsync *d = (FunctionCambioVsync *)datos;
    cambiar_vsync(d->renderer, d->bool_vsync, d->ultimo_clic, d->COOLDOWN_BOTON);
}

void accion_cargar_escena(void *datos)
{
    FunctionCargarEscena *d = (FunctionCargarEscena *)datos;
    Uint64 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - *d->ultimo_clic > d->COOLDOWN_BOTON)
    {
        switch (d->escena_id)
        {
        case 1:
            *d->escena = cargar_escena_1(d->arena, d->modelos_globales, d->n_modelos_globales);
            break;
        }
        *d->ultimo_clic = tiempo_actual;
    }
}

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Escena **escena_actual, Arena *arena_escena, Modelo **modelos_globales)
{
    ui->COOLDOWN_BOTON = 200;
    ui->ultimo_clic = 0;
    ui->bool_vsync = true;

    ui->params_vsync.renderer = renderer;
    ui->params_vsync.bool_vsync = &ui->bool_vsync;
    ui->params_vsync.ultimo_clic = &ui->ultimo_clic;
    ui->params_vsync.COOLDOWN_BOTON = ui->COOLDOWN_BOTON;

    ui->params_escena.escena = escena_actual;
    ui->params_escena.ultimo_clic = &ui->ultimo_clic;
    ui->params_escena.COOLDOWN_BOTON = ui->COOLDOWN_BOTON;
    ui->params_escena.escena_id = 1;
    ui->params_escena.arena = arena_escena;
    ui->params_escena.modelos_globales = modelos_globales;
    ui->params_escena.n_modelos_globales = TOTAL_MODELOS;

    ui->n_botones = 2;    
    ui->botones = (Boton *)arena_push(arena_ui, sizeof(Boton) * ui->n_botones);

    ui->botones[0] = (Boton){20, 50, 140, 30, {100, 100, 100, 255}, "VSYNC ON/OFF", accion_cambiar_vsync, &ui->params_vsync};
    ui->botones[1] = (Boton){20, 90, 140, 30, {100, 100, 100, 255}, "CARGAR ESCENA 1", accion_cargar_escena, &ui->params_escena};

    arena_reporte(arena_ui, "DESPUES DE INICIALIZAR UI");
}

void ui_actualizar(UI *ui)
{
    ui_comenzar_frame();
}

void ui_renderizar(UI *ui, SDL_Renderer *renderer)
{
    for (int i = 0; i < ui->n_botones; i++)
    {
        ui_dibujar_boton(renderer, &ui->botones[i], ui->botones[i].params);
    }
    gestionar_cursor_raton();
}

#endif