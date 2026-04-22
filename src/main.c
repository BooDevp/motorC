#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>

#include "core/gestion_memoria.h"
#include "graphics/layout.h"

// Escenas
#include "graphics/escenas/escena1.h"
#include "graphics/loader_obj.h"

#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la cámara
#define DISTANCIA_CAMARA 1.0f
#define ZOOM 1.0f

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    GestionMemoria gestion_memoria;
    init_app_memory(&gestion_memoria);

    Modelo **modelos_globales = inicializar_catalogo_modelos(&gestion_memoria.arena_objects);

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer("Motor 3D - Viewport Limpio", VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderVSync(renderer, true);

    bool corriendo = true;
    SDL_Event ev;

    Layout layout;
    calcular_layout(&layout, VENTANA_ANCHO, VENTANA_ALTO, ZOOM);

    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo = tiempo_ahora;
    float dt = 0.0f;

    Escena *escena_actual = cargar_escena_1(&gestion_memoria.arena_escena, modelos_globales, TOTAL_MODELOS);

    while (corriendo)
    {
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
                corriendo = false;
        }

        // LIMPIEZA TOTAL DE LA VENTANA
        SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.fondo.r, TEMA_DEFAULT.fondo.g, TEMA_DEFAULT.fondo.b, 255);
        SDL_RenderClear(renderer);

        tiempo_ultimo = tiempo_ahora;
        tiempo_ahora = SDL_GetTicks();
        dt = (tiempo_ahora - tiempo_ultimo) / 1000.0f;

        actualizar_escena(escena_actual, dt);

        pintar_escena(escena_actual, renderer, DISTANCIA_CAMARA, &layout);

        pintar_layout(renderer, &layout);

        // Barra inferior
        SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
        SDL_RenderDebugText(renderer, (float)layout.menu_w + 20, 20, "VIEWPORT: ESTATICO");
        SDL_RenderDebugText(renderer, 20, (float)VENTANA_ALTO - (layout.barra_h / 2) - 4, "SISTEMA: LISTO | ASSET_VAL: 50.000 EUR");

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}