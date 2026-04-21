#ifndef TIPOS_H
#define TIPOS_H

#include "graphics/scene.h"

typedef struct {
    Escena **escena_actual;
    Arena *arena_escena;
    Modelo **modelos_globales;
} Mundo;

typedef struct {
    int ancho, alto;
    float zoom;
    float distancia_camara;
    Uint64 *tiempo_fps;
} VentanaInfo;

#endif