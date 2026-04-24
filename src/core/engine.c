#include "core/engine.h"

bool engine_init(Engine *e, const char *titulo, int w, int h)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;

    if (!SDL_CreateWindowAndRenderer(titulo, w, h, 0, &e->window, &e->renderer))
    {
        return false;
    }

    SDL_SetRenderDrawBlendMode(e->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderVSync(e->renderer, true);

    e->corriendo = true;
    e->tiempo_ahora = SDL_GetTicks();
    e->tiempo_ultimo = e->tiempo_ahora;
    e->dt = 0.0f;

    return true;
}

void engine_actualizar_dt(Engine *e)
{
    e->tiempo_ultimo = e->tiempo_ahora;
    e->tiempo_ahora = SDL_GetTicks();
    e->dt = (float)(e->tiempo_ahora - e->tiempo_ultimo) / 1000.0f;
}

void engine_limpiar(Engine *e)
{
    SDL_DestroyRenderer(e->renderer);
    SDL_DestroyWindow(e->window);
    SDL_Quit();
}