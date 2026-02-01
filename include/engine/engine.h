#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>

// Evitamos incluir sokol_gfx múltiples veces si incluimos engine.h en varios sitios
#define SOKOL_GLCORE 
#include "external/sokol_gfx.h"

typedef struct {
    SDL_Window* window;
    SDL_GLContext gl_context;
    sg_pass_action pass_action;
    bool running;
    int width;
    int height;
    
    // Para el contador de FPS
    uint64_t last_time;
    int frames;
} Engine;

// Función para arrancar el motor
static bool engine_init(Engine* en, const char* title, int w, int h) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    en->window = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL);
    if (!en->window) return false;

    en->gl_context = SDL_GL_CreateContext(en->window);
    SDL_GL_MakeCurrent(en->window, en->gl_context);

    // VSync activado
    SDL_GL_SetSwapInterval(1);

    // Inicializar Sokol GFX
    sg_setup(&(sg_desc){0});

    // Color de limpieza por defecto (Gris PS1)
    en->pass_action = (sg_pass_action) {
        .colors[0] = { 
            .load_action = SG_LOADACTION_CLEAR, 
            .clear_value = { 0.1f, 0.1f, 0.1f, 1.0f } 
        }
    };

    en->width = w;
    en->height = h;
    en->running = true;
    en->last_time = SDL_GetTicks();
    en->frames = 0;

    printf("Motor iniciado: %dx%d | OpenGL 3.3 | VSync ON\n", w, h);
    return true;
}

// Función para actualizar eventos y contador de FPS
static void engine_update(Engine* en) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) en->running = false;
    }

    SDL_GetWindowSizeInPixels(en->window, &en->width, &en->height);

    // Contador de FPS
    en->frames++;
    uint64_t now = SDL_GetTicks();
    if (now - en->last_time >= 1000) {
        printf("FPS: %d\n", en->frames);
        en->frames = 0;
        en->last_time = now;
    }
}

// Función para cerrar todo
static void engine_cleanup(Engine* en) {
    sg_shutdown();
    SDL_GL_DestroyContext(en->gl_context);
    SDL_DestroyWindow(en->window);
    SDL_Quit();
    printf("Motor apagado correctamente.\n");
}

#endif