#ifndef STYLE_H
#define STYLE_H

// Librerías SDL3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct
{
    SDL_Color fondo;
    SDL_Color modelo;
    SDL_Color marco;
} Tema;

static const Tema TEMA_DEFAULT = {
    .fondo = {19, 16, 23, 255},
    .modelo = {135, 132, 178, 255},
    .marco = {121, 117, 131, 255},
};

#endif