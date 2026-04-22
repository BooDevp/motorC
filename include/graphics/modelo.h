#ifndef MODELO_H
#define MODELO_H

#include "core/gestor_memoria.h"
#include "core/math_3d.h"

#include <SDL3/SDL.h>
#include <ctype.h>
#include <float.h>
#include <math.h>

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