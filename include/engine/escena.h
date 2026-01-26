#ifndef ESCENA_H
#define ESCENA_H

#include "model.h"
#include "arena.h"

typedef struct {
    Modelo *modelos;
    int cantidad;
    int capacidad;
} Escena;

/**
 * Inicializa la estructura de la escena pidiendo espacio a la arena
 */
static inline Escena crear_escena(Arena *arena, int capacidad_maxima) {
    Escena escena = {0};
    escena.capacidad = capacidad_maxima;
    escena.modelos = (Modelo *)arena_push(arena, sizeof(Modelo) * capacidad_maxima);
    escena.cantidad = 0;
    return escena;
}

/**
 * Añade un modelo a la escena cargándolo desde un archivo
 */
static inline Modelo* escena_añadir_modelo(Escena *escena, Arena *arena, const char *ruta) {
    if (escena->cantidad >= escena->capacidad) {
        debug_log("ERROR: Escena llena, no se puede añadir %s", ruta);
        return NULL;
    }

    Modelo *nuevo = &escena->modelos[escena->cantidad];
    if (cargar_modelo(nuevo, arena, ruta)) {
        escena->cantidad++;
        return nuevo;
    }
    
    return NULL;
}

/**
 * Crea una instancia (copia) de un modelo que ya existe en la escena
 * Útil para bosques, hordas, etc.
 */
static inline Modelo* escena_instanciar_modelo(Escena *escena, int indice_original) {
    if (escena->cantidad >= escena->capacidad || indice_original >= escena->cantidad) {
        return NULL;
    }

    Modelo *instancia = &escena->modelos[escena->cantidad];
    *instancia = escena->modelos[indice_original]; // Copia bit a bit (mismo VAO/VBO)
    escena->cantidad++;
    return instancia;
}

#endif