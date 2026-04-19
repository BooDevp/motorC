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

// Configuraciones de pantalla
#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la camara
#define DISTANCIA_CAMARA 1.0
#define ZOOM 1.0

// Configuracion memoria
#define MB(x) ((size_t)(x) * 1024 * 1024)
#define ARENA_SIZE_MB_OBJECTS 40
#define ARENA_SIZE_MB_SCENE 24

// --- MAIN ---
int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    ui_init();

    Arena arena_objects;
    arena_inicializar(&arena_objects, MB(ARENA_SIZE_MB_OBJECTS), "OBJETOS");

    // Cargar modelos globales
    Modelo **modelos_globales = (Modelo **)arena_push(&arena_objects, sizeof(Modelo *) * TOTAL_MODELOS);
    for (int i = 0; i < TOTAL_MODELOS; i++)
    {
        modelos_globales[i] = get_modelo_obj(&arena_objects, rutas_modelos_globales[i]);
        if (modelos_globales[i])
        {
            calcular_centros(modelos_globales[i]->vertices, modelos_globales[i]->n_puntos, &modelos_globales[i]->cx, &modelos_globales[i]->cy, &modelos_globales[i]->cz);
        }
    }
    arena_reporte(&arena_objects, "DESPUES DE CARGAR MODELOS");

    Arena arena_escena;
    arena_inicializar(&arena_escena, MB(ARENA_SIZE_MB_SCENE), "ESCENA");

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer("Motor 3D", VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Habilitar transparencia globalmente

    // ESTADO DEL MOTOR
    Escena *escena_actual = NULL;
    float escala = (VENTANA_ANCHO / 2.0f) * ZOOM;

    // VARIABLES DE FPS Y TIEMPO
    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo = 0;
    Uint64 tiempo_anterior_fps = SDL_GetTicks();
    Uint64 frames_contados = 0;
    float fps_actuales = 0;
    float dt = 0;
    char texto_fps[64] = "Iniciando...";

    // BOTONES Y UI
    Uint64 ultimo_clic = 0;
    const Uint64 COOLDOWN_BOTON = 200;
    bool bool_vsync = true;

    FunctionCambioVsync params_vsync = {renderer, &bool_vsync, &ultimo_clic, COOLDOWN_BOTON};
    Boton btn_vsync = {20, 50, 140, 30, {100, 100, 100, 255}, "VSYNC ON/OFF", accion_cambiar_vsync, &params_vsync};

    FunctionCargarEscena params_escena = {&escena_actual, &ultimo_clic, COOLDOWN_BOTON, 1, &arena_escena, modelos_globales, TOTAL_MODELOS};
    Boton btn_cargar_escena = {20, 90, 140, 30, {100, 100, 100, 255}, "CARGAR ESCENA 1", accion_cargar_escena, &params_escena};

    SDL_SetRenderVSync(renderer, bool_vsync);

    // --- BUCLE PRINCIPAL ---
    bool corriendo = true;
    SDL_Event ev;

    while (corriendo)
    {
        ui_comenzar_frame();
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
                corriendo = false;
        }

        // Delta Time
        tiempo_ultimo = tiempo_ahora;
        tiempo_ahora = SDL_GetTicks();
        dt = (tiempo_ahora - tiempo_ultimo) / 1000.0f;

        // --- LÓGICA (Actualizar posiciones y rotaciones) ---
        if (escena_actual != NULL)
        {
            actualizar_escena(escena_actual, dt);
        }

        // --- RENDERIZADO ---
        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
        SDL_RenderClear(renderer);

        if (escena_actual != NULL)
        {
            pintar_escena(escena_actual, renderer, DISTANCIA_CAMARA, VENTANA_ANCHO, VENTANA_ALTO, escala);
        }

        // --- UI Y ESTADÍSTICAS ---
        calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), &tiempo_anterior_fps);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10, 10, texto_fps);
        ui_dibujar_boton(renderer, &btn_vsync, btn_vsync.params);
        ui_dibujar_boton(renderer, &btn_cargar_escena, btn_cargar_escena.params);
        SDL_RenderPresent(renderer);

        // --- LÓGICA ---
        gestionar_cursor_raton();
    }

    // --- LIMPIEZA ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(arena_objects.base);
    free(arena_escena.base);

    return 0;
}