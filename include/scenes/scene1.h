#ifndef SCENE1_H
#define SCENE1_H

#include "scene.h"
#include "modelos_id.h"

static Escena *cargar_escena_1(Arena *arena, Modelo **modelos_globales, int n_modelos)
{
    Instancia mis_modelos[] = {
        {
            .modelo_index = MODELO_CUBO,
            .posicion = {-0.5f, 0.0f, 0.0f},
            .rotacion = {0.0f, 0.0f, 0.0f},
            .vel_rotacion = {0.0f, 0.0f, 0.0f}, 
            .escala = 1.0f,
            .color_r = 255, .color_g = 255, .color_b = 255,
            .transparencia = 255
        },
        {
            .modelo_index = MODELO_HANDPOPUS,
            .posicion = {0.5f, 0.0f, 0.0f},
            .rotacion = {0.0f, 45.0f, 0.0f},
            .vel_rotacion = {0.0f, 100.0f, 0.0f}, 
            .escala = 1.0f,
            .color_r = 255, .color_g = 255, .color_b = 255,
            .transparencia = 255
        },        
    };

    int total = sizeof(mis_modelos) / sizeof(mis_modelos[0]);
        
    return cargar_escena_desde_config(arena, mis_modelos, total, modelos_globales);
}

#endif