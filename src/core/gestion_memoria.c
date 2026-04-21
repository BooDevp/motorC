#define SDL_MAIN_HANDLED
#include "core/gestion_memoria.h"

void arena_reporte(Arena *a, const char *contexto)
{
    float porcentaje = (a->usado * 100.0f) / a->capacidad;
    size_t libre = a->capacidad - a->usado;

    printf("\n--- REPORTE [%s] -> %s ---\n", a->nombre, contexto);
    printf("Estado: %.4f%% ocupado\n", porcentaje);
    printf("Usado:  %zu bytes\n", a->usado);
    printf("Libre:  %zu bytes\n", libre);
    printf("Total:  %zu bytes\n", a->capacidad);
    printf("--------------------------------------\n\n");
}

void arena_inicializar(Arena *a, size_t size_arena, const char *nombre_arena)
{
    a->capacidad = size_arena;
    a->usado = 0;
    a->nombre = nombre_arena;
    a->base = (uint8_t *)malloc(size_arena);

    if (!a->base)
    {
        printf("CRITICAL ERROR: No se pudo asignar la Arena [%s] de %zu bytes\n", nombre_arena, size_arena);
    }
    else
    {
        arena_reporte(a, "INICIALIZADA");
    }
}

void init_app_memory(Arena *arena_objects, Arena *arena_escena, Arena *arena_ui)
{
    arena_inicializar(arena_objects, MB(40), "OBJETOS");
    arena_inicializar(arena_escena, MB(24), "ESCENA");
    arena_inicializar(arena_ui, MB(8), "LAYOUT/UI");
}

void *arena_push(Arena *a, size_t size_pedido)
{
    size_t alineado = (size_pedido + 7) & ~7;

    if (a->usado + alineado <= a->capacidad)
    {
        void *puntero = a->base + a->usado;
        a->usado += alineado;
        return puntero;
    }

    printf("!!! ERROR: Arena [%s] sin espacio !!!\n", a->nombre);
    return NULL;
}

void arena_reset(Arena *a)
{
    a->usado = 0;
    arena_reporte(a, "RESET");
}