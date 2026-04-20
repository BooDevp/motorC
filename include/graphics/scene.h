#ifndef SCENE_H
#define SCENE_H

#include "modelo.h"
#include "core/gestion_memoria.h"
#include "core/math_3d.h"
#include "config/style.h"

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
} Escena;

Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales);
void actualizar_escena(Escena *escena, float dt);
void pintar_escena(Escena *escena, SDL_Renderer *renderer, float distancia_camara, int area_w, int area_h, float escala_global, int offset_x, int offset_y);

#endif