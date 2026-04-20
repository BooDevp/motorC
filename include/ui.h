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

// Layout de la interfaz
#define UI_MENU_RATIO 0.25f  // 25% de ancho para el menú lateral
#define UI_BARRA_RATIO 0.05f // 10% de alto para la barra inferior

// Variables internas
static bool g_hover_any_btn = false;
static SDL_Cursor *cursor_mano = NULL;
static SDL_Cursor *cursor_normal = NULL;

Uint64 frames_contados = 0;
float fps_actuales = 0, dt = 0;
char texto_fps[64] = "Iniciando...";

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
    
    int barra_inferior_h;
    int menu_lateral_w;
    int area_util_h;
    int juego_w;
    int juego_h;
    int juego_offset_x;
    int juego_offset_y;
    float escala_juego;

    int ventana_ancho;
    int ventana_alto;

    Uint64 *tiempo_anterior_fps;
} UI;

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

void ui_inicializar(UI *ui, Arena *arena_ui, SDL_Renderer *renderer, Escena **escena_actual, Arena *arena_escena, Modelo **modelos_globales, int ventana_ancho, int ventana_alto, float zoom, Uint64 *tiempo_anterior_fps)
{

    // Cálculo del Layout Dinámico
    ui->barra_inferior_h = (int)(ventana_alto * UI_BARRA_RATIO);
    ui->menu_lateral_w = (int)(ventana_ancho * UI_MENU_RATIO);
    ui->area_util_h = ventana_alto - ui->barra_inferior_h;
    ui->juego_w = ventana_ancho - ui->menu_lateral_w;
    ui->juego_h = ui->area_util_h;
    ui->juego_offset_x = ui->menu_lateral_w;
    ui->juego_offset_y = 0;
    ui->escala_juego = (ui->juego_w / 2.0f) * zoom;

    // Valores de la ventana
    ui->ventana_alto = ventana_alto;
    ui->ventana_ancho = ventana_ancho;

    ui->tiempo_anterior_fps = tiempo_anterior_fps;

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

void ui_pintar_marcos(UI *ui, SDL_Renderer *renderer)
{
    // --- MARCOS DIEGÉTICOS ---
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.marco.r, TEMA_DEFAULT.marco.g, TEMA_DEFAULT.marco.b, TEMA_DEFAULT.marco.a);
    // Línea horizontal
    SDL_RenderLine(renderer, 0, ui->area_util_h, ui->ventana_ancho, ui->area_util_h);
    // Línea vertical
    SDL_RenderLine(renderer, ui->menu_lateral_w, 0, ui->menu_lateral_w, ui->area_util_h);
}

void ui_pintar_textos(UI *ui, SDL_Renderer *renderer)
{
    // --- TEXTOS ---
    calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), ui->tiempo_anterior_fps);
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, TEMA_DEFAULT.modelo.a);
    SDL_RenderDebugText(renderer, 10, 10, texto_fps);

    char info_status[128];
    SDL_snprintf(info_status, sizeof(info_status), "OS_CORE: ACTIVE | SYSTEM_VAL: %.2f EUR", 50000.0f);
    
    // Texto centrado en la barra que ahora es completa
    SDL_RenderDebugText(renderer, 20, ui->ventana_alto - (ui->barra_inferior_h / 2) - 4, info_status);
}

void ui_renderizar(UI *ui, SDL_Renderer *renderer)
{
    ui_pintar_marcos(ui, renderer);
    ui_pintar_textos(ui, renderer);
    for (int i = 0; i < ui->n_botones; i++)
    {
        ui_dibujar_boton(renderer, &ui->botones[i], ui->botones[i].params);
    }
    gestionar_cursor_raton();
}

#endif