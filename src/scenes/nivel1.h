#ifndef NIVEL_1_H
#define NIVEL_1_H

#include "engine/escena.h"

// Escena 1
static inline void cargar_escena_nivel_1(Escena *escena, Arena *arena)
{
    // Limpiar/Inicializar la estructura de la escena
    // (Asumimos que la escena ya fue creada con capacidad suficiente)
    escena->cantidad = 0;

    Modelo *h1 = escena_añadir_modelo(escena, arena, "assets/models/Icecream.obj");
    if (h1)
        h1->posicion[0] = -1.5f;

    Modelo *h2 = escena_instanciar_modelo(escena, 0);
    if (h2)
        h2->posicion[0] = 1.5f;

    Modelo *cubo = escena_añadir_modelo(escena, arena, "assets/models/cubo.obj");
    if (cubo)
    {
        cubo->posicion[1] = -2.0f; // Suelo
        cubo->escala[0] = 10.0f;
        cubo->escala[2] = 10.0f;
    }

    debug_log("Nivel 1 cargado: %d modelos", escena->cantidad);
}

#endif