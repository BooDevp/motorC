#ifndef ESCENA_H
#define ESCENA_H

#include "model.h"
#include "arena.h"

typedef struct
{
    Modelo *modelos;
    int cantidad;
    int capacidad;
} Escena;

/**
 * Inicializa la estructura de la escena pidiendo espacio a la arena
 */
static inline Escena crear_escena(Arena *arena, int capacidad_maxima)
{
    Escena escena = {0};
    escena.capacidad = capacidad_maxima;
    escena.modelos = (Modelo *)arena_push(arena, sizeof(Modelo) * capacidad_maxima);
    escena.cantidad = 0;
    return escena;
}

/**
 * Añade un modelo a la escena cargándolo desde un archivo
 * shader_id: 0 para usar el shader global, o un ID válido para shader custom
 */
static inline Modelo *escena_añadir_modelo(Escena *escena, Arena *arena, const char *ruta, GLuint shader_id)
{
    if (escena->cantidad >= escena->capacidad)
    {
        debug_log("ERROR: Escena llena, no se puede añadir %s", ruta);
        return NULL;
    }

    Modelo *nuevo = &escena->modelos[escena->cantidad];
    if (cargar_modelo(nuevo, arena, ruta))
    {
        nuevo->shader = shader_id;

        // OPTIMIZACIÓN: Cachear la location del uniform MVP
        if (shader_id != 0)
        {
            nuevo->mvp_location = glGetUniformLocation(shader_id, "uMVP");
            if (nuevo->mvp_location == -1)
            {
                debug_log("ADVERTENCIA: Shader custom (ID %u) no tiene uniform 'uMVP' o está optimizado.", shader_id);
            }
        }

        escena->cantidad++;
        return nuevo;
    }

    return NULL;
}

/**
 * Crea una instancia (copia) de un modelo que ya existe en la escena
 * Útil para modelos repetidos.
 */
static inline Modelo *escena_instanciar_modelo(Escena *escena, int indice_original)
{
    if (escena->cantidad >= escena->capacidad || indice_original >= escena->cantidad)
    {
        return NULL;
    }

    Modelo *instancia = &escena->modelos[escena->cantidad];
    *instancia = escena->modelos[indice_original]; // Copia bit a bit (mismo VAO/VBO)
    escena->cantidad++;
    return instancia;
}

#endif