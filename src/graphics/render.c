#define SDL_MAIN_HANDLED
#include "graphics/render.h"

void cambiar_vsync(SDL_Renderer *renderer, bool *habilitar, Uint64 *ultimo_clic, Uint64 cooldown)
{
    Uint64 ahora = SDL_GetTicks();
    if (ahora - *ultimo_clic < cooldown)
        return;

    *habilitar = !(*habilitar);

    // Aplicamos el cambio al renderer
    SDL_SetRenderVSync(renderer, *habilitar ? true : false);

    *ultimo_clic = ahora;
    printf("VSync: %s\n", *habilitar ? "Activado" : "Desactivado");
}