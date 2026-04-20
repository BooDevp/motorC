#ifndef MODELO_H
#define MODELO_H

#include "gestion_memoria.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include "math_3d.h"
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

void normalizacion_objeto_centrar(Modelo *f);

// Cargar modelo desde archivo .obj (Blender)
extern Modelo *get_modelo_obj(Arena *arena, const char *ruta);

#endif