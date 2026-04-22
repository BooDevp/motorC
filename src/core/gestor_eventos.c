#include "core/gestor_eventos.h"

bool eventos(SDL_Event *ev) {
    if (ev->type == SDL_EVENT_QUIT) {
        return false;
    }
    
    if (ev->type == SDL_EVENT_KEY_DOWN) {
        if (ev->key.key == SDLK_ESCAPE) return false;
    }

    return true;
}