// Librerías SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// Mis librerías
#include "core/math_3d.h"
#include "core/utils.h"
#include "ui.h"
#include "core/gestion_memoria.h"
#include "config/modelos_id.h"
#include "graphics/scene.h"

// Configuraciones de ventana base
#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la cámara
#define DISTANCIA_CAMARA 1.0f
#define ZOOM 1.0f

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;    

    // Inicialización de Memoria (Arenas)
    Arena arena_objects, arena_escena, arena_ui;
    init_app_memory(&arena_objects, &arena_escena, &arena_ui);

    // Carga de recursos globales
    Modelo **modelos_globales = (Modelo **)arena_push(&arena_objects, sizeof(Modelo *) * TOTAL_MODELOS);
    for (int i = 0; i < TOTAL_MODELOS; i++)
    {
        modelos_globales[i] = get_modelo_obj(&arena_objects, rutas_modelos_globales[i]);
        if (modelos_globales[i])
        {
            calcular_centros(modelos_globales[i]->vertices, modelos_globales[i]->n_puntos,
                             &modelos_globales[i]->cx, &modelos_globales[i]->cy, &modelos_globales[i]->cz);
        }
    }
    arena_reporte(&arena_objects, "RECURSOS GLOBALES CARGADOS");

    // Ventana y Renderer
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer("Motor 3D", VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);    

    // Variables de tiempo
    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo, tiempo_anterior_fps = tiempo_ahora;
    float dt = 0.0f;

    // Inicialización de UI
    Escena *escena_actual = NULL;
    UI ui;
    ui_inicializar(&ui, &arena_ui, renderer, &escena_actual, &arena_escena, modelos_globales, VENTANA_ANCHO, VENTANA_ALTO, ZOOM, &tiempo_anterior_fps);
    SDL_SetRenderVSync(renderer, ui.bool_vsync);

    // --- BUCLE PRINCIPAL ---
    bool corriendo = true;
    SDL_Event ev;

    while (corriendo)
    {
        ui_actualizar(&ui);
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
                corriendo = false;
        }

        tiempo_ultimo = tiempo_ahora;
        tiempo_ahora = SDL_GetTicks();
        dt = (tiempo_ahora - tiempo_ultimo) / 1000.0f;

        if (escena_actual != NULL)
            actualizar_escena(escena_actual, dt);

        // --- RENDERIZADO ---
        SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.fondo.r, TEMA_DEFAULT.fondo.g, TEMA_DEFAULT.fondo.b, TEMA_DEFAULT.fondo.a);
        SDL_RenderClear(renderer);

        if (escena_actual != NULL)
        {
            pintar_escena(escena_actual, renderer, DISTANCIA_CAMARA,
                          ui.juego_w, ui.juego_h, ui.escala_juego,
                          ui.juego_offset_x, ui.juego_offset_y);
        }
        
        ui_renderizar(&ui, renderer);
        SDL_RenderPresent(renderer);
    }

    // --- LIMPIEZA ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(arena_objects.base);
    free(arena_escena.base);
    free(arena_ui.base);

    return 0;
}