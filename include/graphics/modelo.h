#ifndef MODELO_H
#define MODELO_H

#include "core/gestion_memoria.h"
#include <SDL3/SDL.h>
#include "core/math_3d.h"
#include <ctype.h>
#include <float.h>

typedef struct
{
    int *vertices;
    int n_vertices;
} Cara;

typedef struct
{
    float *vertices;
    int n_puntos;
    Cara *caras;
    int n_caras;
    float cx, cy, cz;
} Modelo;

void normalizacion_modelo_centrar(Modelo *f);

#endif