#include "core/gestion_memoria.h"

#define ARENA_OBJECTS_MB 40
#define ARENA_ESCENA_MB 24
#define ARENA_UI_MB 8

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

static void arena_inicializar(Arena *a, size_t size_arena, const char *nombre_arena)
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

void init_memoria(GestionMemoria *gestion_memoria)
{
    arena_inicializar(&gestion_memoria->arena_objects, MB(ARENA_OBJECTS_MB), "MODELOS");
    arena_inicializar(&gestion_memoria->arena_escena, MB(ARENA_ESCENA_MB), "ESCENA");
    arena_inicializar(&gestion_memoria->arena_ui, MB(ARENA_UI_MB), "UI");
}

void liberar_memoria(GestionMemoria *gestion_memoria)
{
    free(gestion_memoria->arena_objects.base);
    free(gestion_memoria->arena_escena.base);
    free(gestion_memoria->arena_ui.base);
}