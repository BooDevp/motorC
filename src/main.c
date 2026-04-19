// Librerías SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// Mis librerías
#include "math_3d.h"
#include "utils.h"
#include "ui.h"
#include "gestion_memoria.h"
#include "modelos_id.h"
#include "scene.h"

// Configuraciones de ventana base
#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la cámara
#define DISTANCIA_CAMARA 1.0f
#define ZOOM 1.0f

// Layout de la interfaz
#define UI_MENU_RATIO 0.25f  // 25% de ancho para el menú lateral
#define UI_BARRA_RATIO 0.05f // 10% de alto para la barra inferior

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    ui_init();

    // Inicialización de Memoria (Arenas)
    Arena arena_objects, arena_escena, arena_ui;
    init_app_memory(&arena_objects, &arena_escena, &arena_ui);

    // 2. Carga de recursos globales
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

    // 3. Ventana y Renderer
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer("Motor 3D", VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Cálculo del Layout Dinámico
    int barra_inferior_h = (int)(VENTANA_ALTO * UI_BARRA_RATIO);
    int menu_lateral_w = (int)(VENTANA_ANCHO * UI_MENU_RATIO);    
    int area_util_h = VENTANA_ALTO - barra_inferior_h;
    int juego_w = VENTANA_ANCHO - menu_lateral_w;
    int juego_h = area_util_h;
    int juego_offset_x = menu_lateral_w;
    int juego_offset_y = 0;
    float escala_juego = (juego_w / 2.0f) * ZOOM;

    // Inicialización de UI
    Escena *escena_actual = NULL;
    UI ui;
    ui_inicializar(&ui, &arena_ui, renderer, &escena_actual, &arena_escena, modelos_globales);
    SDL_SetRenderVSync(renderer, ui.bool_vsync);

    // Variables de tiempo
    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo, tiempo_anterior_fps = tiempo_ahora;
    Uint64 frames_contados = 0;
    float fps_actuales = 0, dt = 0;
    char texto_fps[64] = "Iniciando...";

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
                          juego_w, juego_h, escala_juego,
                          juego_offset_x, juego_offset_y);
        }

        // --- MARCOS DIEGÉTICOS ---
        SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.marco.r, TEMA_DEFAULT.marco.g, TEMA_DEFAULT.marco.b, TEMA_DEFAULT.marco.a);
        
        // Línea horizontal
        SDL_RenderLine(renderer, 0, area_util_h, VENTANA_ANCHO, area_util_h);
        // Línea vertical
        SDL_RenderLine(renderer, menu_lateral_w, 0, menu_lateral_w, area_util_h);

        // --- TEXTOS ---
        calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), &tiempo_anterior_fps);
        SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, TEMA_DEFAULT.modelo.a);
        SDL_RenderDebugText(renderer, 10, 10, texto_fps);

        char info_status[128];
        SDL_snprintf(info_status, sizeof(info_status), "OS_CORE: ACTIVE | SYSTEM_VAL: %.2f EUR", 50000.0f);
        // Texto centrado en la barra que ahora es completa
        SDL_RenderDebugText(renderer, 20, VENTANA_ALTO - (barra_inferior_h / 2) - 4, info_status);

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