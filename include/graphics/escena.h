#ifndef SCENE_H
#define SCENE_H

#include "modelo.h"
#include "core/math_3d.h"
#include "graphics/layout.h"

typedef struct
{
    int modelo_index;
    Modelo *modelo;
    Vec3 posicion;
    Vec3 rotacion;
    Vec3 vel_rotacion;
    float escala;
    Uint8 color_r, color_g, color_b;
    int transparencia;
} Instancia;

typedef struct
{
    Instancia *instancias;
    int n_instancias;

    Modelo **catalogo_referencia;
} Escena;

void actualizar_escena(Escena *escena, float dt);
void pintar_escena(Escena *escena, SDL_Renderer *renderer, float distancia_camara, Layout *layout);
Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales);

#endif