#ifndef GESTION_MEMORIA_H
#define GESTION_MEMORIA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define MB(x) ((size_t)(x) * 1024 * 1024)

typedef struct
{
    size_t capacidad;
    size_t usado;
    uint8_t *base;
    const char *nombre;
} Arena;

void arena_reporte(Arena *a, const char *contexto);
void arena_inicializar(Arena *a, size_t size_arena, const char *nombre_arena);
void init_app_memory(Arena *arena_objects, Arena *arena_escena, Arena *arena_ui);
void *arena_push(Arena *a, size_t size_pedido);
void arena_reset(Arena *a);

#endif