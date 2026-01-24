#ifndef GESTION_MEMORIA_H
#define GESTION_MEMORIA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    size_t capacidad;
    size_t usado;
    uint8_t *base;
} Arena;

void arena_reporte(Arena *a, const char *etiqueta);
void arena_inicializar(Arena *a, size_t size_arena);
void *arena_push(Arena *a, size_t size_perdido);
void arena_reset(Arena *a);

#endif