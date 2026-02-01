#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Estructura de la Arena: Un bloque de memoria simple y lineal
typedef struct
{
    uint8_t *base; // Puntero al inicio del bloque
    size_t size;   // Tamaño total reservado
    size_t offset; // Cuánto hemos gastado ya
} MemoryArena;

// Inicializa la arena reservando un bloque grande de una vez
static bool arena_init(MemoryArena *a, size_t size)
{
    a->base = (uint8_t *)malloc(size);
    if (!a->base)
        return false;

    a->size = size;
    a->offset = 0;
    return true;
}

// Reserva espacio dentro de la arena (el "malloc" del motor)
static void *arena_alloc(MemoryArena *a, size_t size)
{
    // Alineación a 8 bytes (importante para CPUs modernas)
    size_t aligned_size = (size + 7) & ~7;

    if (a->offset + aligned_size <= a->size)
    {
        void *ptr = &a->base[a->offset];
        a->offset += aligned_size;
        return ptr;
    }

    fprintf(stderr, "ERROR: Memoria insuficiente en la Arena!\n");
    return NULL;
}

// Limpia la arena (resetea el puntero, no borra los datos, muy rápido)
static void arena_reset(MemoryArena *a)
{
    a->offset = 0;
}

// Libera la memoria del sistema
static void arena_free(MemoryArena *a)
{
    free(a->base);
    a->base = NULL;
    a->size = 0;
    a->offset = 0;
}

#endif