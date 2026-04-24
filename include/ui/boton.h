#ifndef BOTON_H
#define BOTON_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct
{
    SDL_FRect rect;
    const char *texto;
    void (*accion)(void);
    bool is_hovered;
    bool is_pressed;
} Boton;

void boton_crear(Boton *b, float x, float y, float w, float h, const char *texto, void (*func)(void));
void boton_actualizar(Boton *b, float mouse_x, float mouse_y);
void boton_dibujar(SDL_Renderer *renderer, Boton *b);
void boton_intentar_clic(Boton *b, float mouse_x, float mouse_y);

#endif