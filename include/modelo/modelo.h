#ifndef MODELO_H
#define MODELO_H

#include "memoria/gestion_memoria.h"
#include "utils/utils.h"
#include <SDL3/SDL.h>
#include "math_3d/math_3d.h"

typedef struct
{
    float *vertices;
    int *aristas;
    int n_puntos;
    int n_aristas;
    float cx, cy, cz;
} Modelo;

Modelo *get_modelo_obj(Arena *arena, const char *ruta);
void cargar_modelo(Modelo **modelo_actual, Uint64 *ultimo_clic, const Uint64 COOLDOWN_BOTON, Arena *arena);
void normalizacion_objeto_centrar(Modelo *f);
void pintar_modelo(Modelo *f, SDL_Renderer *renderer, float angulo, float distancia_camara, int ventana_ancho, int ventana_alto, float escala);

#endif