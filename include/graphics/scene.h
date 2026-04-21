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

typedef struct
{
    int ventana_ancho, ventana_alto;
    int menu_lateral_w;
    int barra_inferior_h;
    int area_util_h;
    int juego_w, juego_h;
    int juego_offset_x, juego_offset_y;
    float escala_juego;    

    Uint64 *tiempo_anterior_fps;    
} Layout;

Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales);
void actualizar_escena(Escena *escena, float dt);
void pintar_escena(Escena *escena, SDL_Renderer *renderer, float distancia_camara, Layout *layout);

#endif