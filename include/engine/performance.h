#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <SDL3/SDL.h>
#include <stdio.h>

typedef struct {
    uint64_t last_time;
    uint64_t frame_count;
    float fps;
    float timer;
} PerfCounter;

// Inicializa el contador
static inline PerfCounter perf_init() {
    PerfCounter p = {0};
    p.last_time = SDL_GetTicks();
    return p;
}

// Actualiza los FPS y el título de la ventana cada segundo
static inline void perf_update(PerfCounter *p, SDL_Window *window, float delta_time) {
    p->frame_count++;
    p->timer += delta_time;

    // Actualizar cada 1.0 segundos para que sea legible
    if (p->timer >= 1.0f) {
        p->fps = (float)p->frame_count / p->timer;
        
        char title[128];
        snprintf(title, sizeof(title), "Motor OpenGL | FPS: %.2f | Delta: %.4f ms", 
                 p->fps, delta_time * 1000.0f);
        
        SDL_SetWindowTitle(window, title);

        // Resetear para el siguiente segundo
        p->timer = 0.0f;
        p->frame_count = 0;
    }
}

#endif