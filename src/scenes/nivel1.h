#ifndef NIVEL_1_H
#define NIVEL_1_H

#include "engine/escena.h"

#define NUM_MODELOS 1

// Escena 1
static inline Escena cargar_escena_nivel_1(Arena *arena)
{
    // Creamos la escena con capacidad
    Escena escena = crear_escena(arena, NUM_MODELOS);

    Modelo *h1 = escena_añadir_modelo(&escena, arena, "assets/models/Cigarro.obj");
    if (h1)
    {
        h1->rotacion[1] = 0.0f;
        h1->posicion[2] = 1.5f;
    }

    debug_log("Nivel 1 cargado: %d modelos", escena.cantidad);

    return escena;
}

#endif