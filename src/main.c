/**
 * Motor Gráfico Simple con OpenGL 3.3 y SDL3 - VERSIÓN WINDOWS
 */

// C
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// EXTERNAL
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define GL_IMPLEMENTATION
#include "external/gl_headers.h"

// ENGINE
#include "engine/arena.h"
#include "engine/shader.h"
#include "engine/appstate.h"
#include "engine/engine.h"
#include "engine/camara.h"
#include "engine/model.h"
#include "engine/render.h"
#include "engine/escena.h"
#include "engine/postprocesado.h"
#include "engine/performance.h"
#include "engine/texture.h"

// Escenas
#include "scenes/nivel1.h"

// ============================================================================
// CONSTANTES CONFIGURABLES
// ============================================================================
#define ARENA_SIZE_MB 10

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char *argv[])
{
    (void)argc; // Silenciar el warning
    (void)argv; // Silenciar el warning
    printf("========================================\n");
    printf("MOTOR OPENGL 3.3 (Windows)\n");
    printf("========================================\n\n");

    AppState app = init_appstate();

    GraphicsState gs = {0};
    SDL_Window *window = NULL;

    // GESTIÓN DE MEMORIA
    Arena arena_escena;
    arena_inicializar(&arena_escena, ARENA_SIZE_MB * 1024 * 1024);

    // INICIALIZAR SDL
    debug_log("Inicializando SDL3...");
    if (!init_sdl(&window))
    {
        return 1;
    }

    // INICIALIZAR OPENGL
    debug_log("Inicializando OpenGL...");
    if (!init_opengl(window, &app))
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // CREAR SHADERS
    debug_log("Creando shaders...");
    gs.program = create_shader_program();
    if (gs.program == 0)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Obtener ubicación del uniform
    gs.mvp_location = glGetUniformLocation(gs.program, "uMVP");
    if (gs.mvp_location == -1)
    {
        debug_log("ADVERTENCIA: Uniform uMVP no encontrado (puede estar optimizado)");
    }
    else
    {
        debug_log("Uniform uMVP ubicado en: %d", gs.mvp_location);
    }

    // CARGAR NIVEL 1
    Escena nivel = cargar_escena_nivel_1(&arena_escena);

    // CONFIGURACIÓN CÁMARA
    Camara mi_camara = crear_camara_defecto();

    // CONFIGURAR MATRICES
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);

    PostProcessSystem pp;
    post_init(&pp, width, height);

    uint64_t last_time = SDL_GetTicks();
    float delta_time = 0.0f;

    PerfCounter perf = perf_init();

    debug_log("\n========================================");
    debug_log("MOTOR LISTO");
    debug_log("Controles:");
    debug_log("  ESC - Salir");
    debug_log("  F1  - Alternar wireframe");
    debug_log("  F2  - Alternar postprocesado");
    debug_log("  F3  - Alternar VSync");
    debug_log("========================================\n");

    // BUCLE PRINCIPAL
    while (app.running)
    {
        SDL_Event event;
        uint64_t current_time = SDL_GetTicks();

        // Convertimos la diferencia de ms a segundos (float)
        delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // Evitar picos si la ventana se congela o se arrastra
        if (delta_time > 0.1f)
            delta_time = 0.1f;

        perf_update(&perf, window, delta_time);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                app.running = false;
                debug_log("Evento QUIT recibido");
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    app.running = false;
                    debug_log("Tecla ESC presionada");
                }

                if (event.key.key == SDLK_F1)
                {
                    app.wireframe = !app.wireframe;
                    debug_log("Wireframe: %s", app.wireframe ? "ON" : "OFF");
                }

                if (event.key.key == SDLK_F2)
                {
                    app.postprocesado = !app.postprocesado;
                    debug_log("Postprocesado: %s", app.postprocesado ? "ON" : "OFF");
                }

                if (event.key.key == SDLK_F3)
                {
                    app.vsync = !app.vsync;
                    if (app.vsync)
                    {
                        SDL_GL_SetSwapInterval(1);
                    }
                    else
                    {
                        SDL_GL_SetSwapInterval(0);
                    }
                    debug_log("VSync: %s", app.vsync ? "ON" : "OFF");
                }
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSizeInPixels(window, &width, &height);
                glViewport(0, 0, width, height);
                debug_log("Ventana redimensionada: %dx%d", width, height);
                post_setup_buffers(&pp, width, height);
                debug_log("Buffers de posprocesado redimensionados: %dx%d", width, height);
            }
        }

        // Limpiar la pantalla y el buffer de profundidad
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Empezamos a dibujar en el Framebuffer (Textura)
        if (app.postprocesado)
            post_begin(&pp);

        // El renderizador se encarga de todo lo visual
        for (int i = 0; i < nivel.cantidad; i++)
        {
            render_frame(&gs, &mi_camara, &nivel.modelos[i], &app, width, height);
        }

        // Actualizar animaciones/transformaciones DESPUÉS del renderizado
        // nivel.modelos[0].rotacion[1] += 35.0f * delta_time;

        // Volvemos al buffer de pantalla y dibujamos el Quad con el efecto
        if (app.postprocesado)
        {
            glUseProgram(pp.program);

            // Ya no usamos glGetUniformLocation aquí, usamos lo que guardamos
            if (pp.timeLoc != -1)
                glUniform1f(pp.timeLoc, SDL_GetTicks() / 1000.0f);

            if (pp.resLoc != -1)
                glUniform2f(pp.resLoc, (float)width, (float)height);

            post_end(&pp);
        }

        SDL_GL_SwapWindow(window);
    }

    // LIMPIEZA
    debug_log("\nFinalizando...");
    cleanup(window, &gs);
    free(arena_escena.base);
    debug_log("Arena liberada!");
    printf("\nAplicación terminada correctamente.\n");

    return 0;
}