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
    const char *nombre; // <--- Ahora la arena tiene identidad
} Arena;

// Ya no necesitas pasarle la etiqueta, usa el nombre interno
static void arena_reporte(Arena *a, const char *contexto)
{
    float porcentaje = (a->usado * 100.0f) / a->capacidad;
    size_t libre = a->capacidad - a->usado;

    // Usamos a->nombre para saber de qué arena hablamos
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
    a->nombre = nombre_arena; // Guardamos el nombre (ej: "OBJETOS" o "ESCENA")
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

static void *arena_push(Arena *a, size_t size_perdido)
{
    size_t alineado = (size_perdido + 7) & ~7;
    
    if (a->usado + alineado <= a->capacidad)
    {
        void *puntero = a->base + a->usado;
        a->usado += alineado;        
        return puntero;
    }

    printf("!!! ERROR: Arena [%s] sin espacio !!!\n", a->nombre);
    return NULL;
}

static void arena_reset(Arena *a)
{
    a->usado = 0;
    arena_reporte(a, "RESET"); // Ahora el reporte te dirá qué arena se reseteó
}

#endif