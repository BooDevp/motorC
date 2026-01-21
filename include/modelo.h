#ifndef MODELO_H
#define MODELO_H

#include "gestion_memoria.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include "math_3d.h"

typedef struct {
    int v1, v2, v3;
} Cara;

typedef struct {
    float *vertices;
    int n_puntos;    
    Cara *caras;
    int n_caras;    
    float cx, cy, cz;
} Modelo;

Modelo *get_modelo_obj(Arena *arena, const char *ruta);
void cargar_modelo(Modelo **modelo_actual, Uint64 *ultimo_clic, const Uint64 COOLDOWN_BOTON, Arena *arena);
void pintar_modelo(Modelo *f, SDL_Renderer *renderer, float *z_buffer, float angulo, float distancia_camara, int ventana_ancho, int ventana_alto, float escala);
void imprimir_info_modelo(Modelo *f);
float* inicializar_zbuffer(Arena *arena, int ancho, int alto);
void limpiar_zbuffer(float *z_buffer, int ancho, int alto);

#endif