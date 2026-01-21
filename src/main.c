// Librerías SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

// Mis librerías
#include "math_3d.h"
#include "utils.h"
#include "ui.h"
#include "gestion_memoria.h"
#include "modelo.h"

// Configuraciones de pantalla
#define VENTANA_ANCHO 800
#define VENTANA_ALTO 600

// Configuraciones de la camara
#define DISTANCIA_CAMARA 1.0
#define ZOOM 1.0

// Configuracion memoria
#define ARENA_SIZE_MB 128

// --- MAIN ---
int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    // Inicializar los cursores
    ui_init();

    // Inicializar presupuesto de la RAM
    Arena arena_escena;
    arena_inicializar(&arena_escena, ARENA_SIZE_MB * 1024 * 1024);

    // Crear ventana y renderer SDL
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer("Motor 3D", VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);    

    // No hay modelo al iniciar
    Modelo *modelo_actual = NULL;

    float escala = (VENTANA_ANCHO / 2.0f) * ZOOM;
    float angulo = 0.0f;
    
    SDL_Event ev;

    Uint64 tiempo_ahora = SDL_GetTicks();
    Uint64 tiempo_ultimo = 0;
    float dt = 0;

    // 90 grados por segundo
    float velocidad_giro = 20.0f;

    Uint64 tiempo_anterior = SDL_GetTicks();
    Uint64 frames_contados = 0;
    float fps_actuales = 0;
    char texto_fps[32] = "Calculando FPS...";

    Uint64 ultimo_clic = 0;
    const Uint64 COOLDOWN_BOTON = 200;    
    bool bool_vsync = true;

    Boton btn_vsync = {
        .x = 20,
        .y = 50,
        .w = 140,
        .h = 30,
        .color = {100, 100, 100, 255},
        .etiqueta = "VSYNC ON/OFF",
        .accion = accion_cambiar_vsync,
        .params = &(FunctionCambioVsync){
            .renderer = renderer,
            .bool_vsync = &bool_vsync,
            .ultimo_clic = &ultimo_clic,
            .COOLDOWN_BOTON = COOLDOWN_BOTON}};

    Boton btn_cambio = {
        .x = 20,
        .y = 90,
        .w = 140,
        .h = 30,
        .color = {100, 100, 100, 255},
        .etiqueta = "CARGAR MODELO",
        .accion = accion_cargar_modelo,
        .params = &(FunctionCargarModelo){
            .modelo = &modelo_actual,
            .ultimo_clic = &ultimo_clic,
            .COOLDOWN_BOTON = COOLDOWN_BOTON,
            .arena = &arena_escena}};

    // Habilitar/desactivar VSync
    SDL_SetRenderVSync(renderer, bool_vsync);

    bool corriendo = true;
    while (corriendo)
    {   
        ui_comenzar_frame();

        // Buzon de eventos
        while (SDL_PollEvent(&ev))
        {
            // El usuario cerrado el programa
            if (ev.type == SDL_EVENT_QUIT)
            {
                corriendo = false;
            }
        }

        tiempo_ultimo = tiempo_ahora;
        tiempo_ahora = SDL_GetTicks();

        // Calculamos la diferencia y la pasamos a segundos (por eso / 1000.0f)
        dt = (tiempo_ahora - tiempo_ultimo) / 1000.0f;

        // Limpiamos lienzo poniendo en negro
        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
        SDL_RenderClear(renderer);

        // Dibujamos la figura
        pintar_modelo(modelo_actual, renderer, angulo, DISTANCIA_CAMARA, VENTANA_ANCHO, VENTANA_ALTO, escala);

        // Dibujamos el texto en pantalla
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10, 10, texto_fps);

        // Dibujamos los botones
        ui_dibujar_boton(renderer, &btn_vsync, btn_vsync.params);
        ui_dibujar_boton(renderer, &btn_cambio, btn_cambio.params);

        // Mostramos pantalla
        SDL_RenderPresent(renderer);

        // Actualizamos ángulo
        angulo += velocidad_giro * dt; // Velocidad de giro con Delta Time

        // Calculamos FPS
        calcular_frames(&fps_actuales, &frames_contados, texto_fps, sizeof(texto_fps), &tiempo_anterior);

        // Cursor raton
        gestionar_cursor_raton();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(arena_escena.base);
    return 0;
}