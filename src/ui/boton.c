#include "ui/boton.h"
#include "config/style.h"

void boton_crear(Boton *b, float x, float y, float w, float h, const char *texto, void (*func)(void)) {
    b->rect = (SDL_FRect){x, y, w, h};
    b->texto = texto;
    b->accion = func;
    b->is_hovered = false;
    b->is_pressed = false;
}

void boton_actualizar(Boton *b, float mouse_x, float mouse_y) {
    b->is_hovered = (mouse_x >= b->rect.x && mouse_x <= b->rect.x + b->rect.w &&
                     mouse_y >= b->rect.y && mouse_y <= b->rect.y + b->rect.h);
}

void boton_intentar_clic(Boton *b, float mouse_x, float mouse_y) {
    if (mouse_x >= b->rect.x && mouse_x <= b->rect.x + b->rect.w &&
        mouse_y >= b->rect.y && mouse_y <= b->rect.y + b->rect.h) {
        
        if (b->accion != NULL) {
            b->accion();
        }
    }
}

void boton_dibujar(SDL_Renderer *renderer, Boton *b) {
    Uint8 alpha = b->is_hovered ? 80 : 40;
    float len = b->is_hovered ? 10.0f : 6.0f;

    // Fondo semi-transparente
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, alpha);
    SDL_RenderFillRect(renderer, &b->rect);

    // Esquinas (Muescas técnicas)
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.modelo.r, TEMA_DEFAULT.modelo.g, TEMA_DEFAULT.modelo.b, 255);
    SDL_FRect r = b->rect;
    
    // Arriba-Izquierda
    SDL_RenderLine(renderer, r.x, r.y, r.x + len, r.y);
    SDL_RenderLine(renderer, r.x, r.y, r.x, r.y + len);
    // Arriba-Derecha
    SDL_RenderLine(renderer, r.x + r.w, r.y, r.x + r.w - len, r.y);
    SDL_RenderLine(renderer, r.x + r.w, r.y, r.x + r.w, r.y + len);
    // Abajo-Izquierda
    SDL_RenderLine(renderer, r.x, r.y + r.h, r.x + len, r.y + r.h);
    SDL_RenderLine(renderer, r.x, r.y + r.h, r.x, r.y + r.h - len);
    // Abajo-Derecha
    SDL_RenderLine(renderer, r.x + r.w, r.y + r.h, r.x + r.w - len, r.y + r.h);
    SDL_RenderLine(renderer, r.x + r.w, r.y + r.h, r.x + r.w, r.y + r.h - len);

    // Texto 
    float text_x = b->is_hovered ? r.x + 15 : r.x + 10;
    SDL_RenderDebugText(renderer, text_x, r.y + (r.h / 2) - 4, b->texto);
}