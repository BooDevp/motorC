/**
 * Gestión de memoria con arena
 * 
 * Este archivo implementa una gestión de memoria simple y eficiente
 * utilizando una arena de memoria. Permite reservar bloques contiguos
 * de memoria de forma rápida y eficiente, ideal para reservar
 * estructuras y datos de forma dinámica.
 *
 * La arena se inicializa con una cantidad de memoria fija, y se puede
 * reservar memoria de forma contigua utilizando la función arena_push.
 * Si la arena se llena, se puede resetear utilizando arena_reset.
 *
 * Ejemplo de uso:
 * 
 * Arena arena;
 * arena_inicializar(&arena, 1024 * 1024); // 1MB
 * 
 * // Reservar memoria para una estructura
 * MiEstructura *estructura = (MiEstructura *)arena_push(&arena, sizeof(MiEstructura));
 * 
 * // Resetear la arena
 * arena_reset(&arena);
 */

#ifndef GESTION_MEMORIA_H
#define GESTION_MEMORIA_H

typedef struct
{
    size_t capacidad;
    size_t usado;
    uint8_t *base;
} Arena;

static inline void arena_reporte(Arena *a, const char *etiqueta)
{
    float porcentaje = (a->usado * 100.0f) / a->capacidad;

    size_t libre = a->capacidad - a->usado;

    printf("\n--- MEMORIA ARENA [%s] ---\n", etiqueta);
    printf("Estado: %.4f%% ocupado\n", porcentaje);
    printf("Usado:  %zu bytes\n", a->usado);
    printf("Libre:  %zu bytes\n", libre);
    printf("Total:  %zu bytes\n", a->capacidad);
    printf("---------------------------\n\n");
}

static inline void arena_inicializar(Arena *a, size_t size_arena)
{
    a->capacidad = size_arena;
    a->usado = 0;
    a->base = (uint8_t *)malloc(size_arena);
    if (!a->base)
    {
        printf("CRITICAL ERROR: No se pudo asignar la Arena de %zu bytes\n", size_arena);
    }
    else
    {
        arena_reporte(a, "INICIALIZACION");
    }
}

static inline void *arena_push(Arena *a, size_t size_perdido)
{
    // Alinear a 8 bytes para que la CPU lea más rápido
    size_t alineado = (size_perdido + 7) & ~7;
    
    if (a->usado + alineado <= a->capacidad)
    {
        void *puntero = a->base + a->usado;
        a->usado += alineado;
        arena_reporte(a, "ARENA PUSH");
        return puntero;
    }

    printf("!!! ERROR: Arena sin espacio !!!\n");
    return NULL;
}

// static inline void arena_reset(Arena *a)
// {
//     a->usado = 0;
//     printf("Arena reseteada (Puntero devuelto a 0).\n");
// }

#endif