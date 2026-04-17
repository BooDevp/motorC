#ifndef RENDER_H
#define RENDER_H

#include <SDL3/SDL.h>
#include <stdio.h>

static void cambiar_vsync(SDL_Renderer *renderer, bool *habilitar, Uint64 *ultimo_clic, Uint64 cooldown)
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

#endif