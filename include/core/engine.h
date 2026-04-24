#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool corriendo;
    
    Uint64 tiempo_ahora;
    Uint64 tiempo_ultimo;
    float dt;
} Engine;

bool engine_init(Engine *e, const char *titulo, int w, int h);
void engine_actualizar_dt(Engine *e);
void engine_limpiar(Engine *e);

#endif