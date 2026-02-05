#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>

// Incluimos nuestro gestor de memoria
#include "memory.h"

// Configuración de Sokol
#define SOKOL_GLCORE
#include "external/sokol_gfx.h"

typedef struct
{
    SDL_Window *window;
    SDL_GLContext gl_context;
    sg_pass_action pass_action;
    MemoryArena main_arena; // La RAM dedicada al juego

    bool running;
    int width;
    int height;

    // Auditoría de rendimiento
    uint64_t last_fps_time;
    int frame_count;
} Engine;

// --- FUNCIONES DEL MOTOR ---

static bool engine_init(Engine *en, const char *title, int w, int h, int mb)
{
    // Convertimos megabytes a bytes internamente
    size_t ram_size = (size_t)mb * 1024 * 1024;

    // 1. Reservar la Arena de Memoria antes que nada
    if (!arena_init(&en->main_arena, ram_size))
    {
        fprintf(stderr, "Fallo al reservar la Arena de Memoria\n");
        return false;
    }

    // 2. Inicializar SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;

    // Configuración OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    en->window = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL);
    if (!en->window)
        return false;

    en->gl_context = SDL_GL_CreateContext(en->window);
    SDL_GL_MakeCurrent(en->window, en->gl_context);

    // VSync (Sincronización con el monitor)
    SDL_GL_SetSwapInterval(1);

    // 3. Inicializar Sokol GFX
    sg_setup(&(sg_desc){0});

    // Acción de limpieza por defecto (Fondo gris oscuro)
    en->pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0.1f, 0.1f, 0.1f, 1.0f}}};

    en->width = w;
    en->height = h;
    en->running = true;
    en->last_fps_time = SDL_GetTicks();
    en->frame_count = 0;

    printf("ENGINE: Iniciado [%dx%d] | RAM Arena: %zu MB\n", w, h, mb);
    return true;
}

static void engine_update(Engine *en)
{
    // Procesar eventos de ventana
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            en->running = false;
    }

    // Actualizar dimensiones (por si se cambia el tamaño de ventana)
    SDL_GetWindowSizeInPixels(en->window, &en->width, &en->height);

    // Contador de FPS interno
    en->frame_count++;
    uint64_t now = SDL_GetTicks();
    if (now - en->last_fps_time >= 1000)
    {
        printf("FPS: %d | Arena Offset: %zu bytes\n", en->frame_count, en->main_arena.offset);
        en->frame_count = 0;
        en->last_fps_time = now;
    }
}

static void engine_cleanup(Engine *en)
{
    sg_shutdown();
    SDL_GL_DestroyContext(en->gl_context);
    SDL_DestroyWindow(en->window);

    // Liberar el bloque de la Arena al final
    arena_free(&en->main_arena);

    SDL_Quit();
    printf("ENGINE: Apagado y memoria liberada.\n");
}

#endif // ENGINE_H