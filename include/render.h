#ifndef RENDER_H
#define RENDER_H

#include <SDL3/SDL.h>
#include <stdio.h>

void cambiar_vsync(SDL_Renderer *renderer, bool *habilitar, Uint64 *ultimo_clic, Uint64 cooldown);

#endif