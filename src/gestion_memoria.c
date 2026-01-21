#include "gestion_memoria.h"

// Muestra por consola el estado actual de la memoria
void arena_reporte(Arena *a, const char *etiqueta)
{
    // Multiplicamos por 100.0 (float) PRIMERO para forzar la precisión decimal
    float porcentaje = (a->usado * 100.0f) / a->capacidad;

    size_t libre = a->capacidad - a->usado;

    printf("\n--- MEMORIA ARENA [%s] ---\n", etiqueta);
    printf("Estado: %.4f%% ocupado\n", porcentaje); // Subimos a 4 decimales para ver el uso real
    printf("Usado:  %zu bytes\n", a->usado);
    printf("Libre:  %zu bytes\n", libre);
    printf("Total:  %zu bytes\n", a->capacidad);
    printf("---------------------------\n\n");
}

// 'static inline' permite que el código viva en el .h sin errores de duplicación
void arena_inicializar(Arena *a, size_t size_arena)
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

void *arena_push(Arena *a, size_t size_perdido)
{
    // Alinear a 8 bytes es un truco para que la CPU lea más rápido
    size_t alineado = (size_perdido + 7) & ~7;
    
    if (a->usado + alineado <= a->capacidad)
    {
        void *puntero = a->base + a->usado;
        a->usado += alineado;
        printf("Pedido: %zu bytes | Disponible: %zu bytes\n", alineado, a->capacidad - a->usado);
        return puntero;
    }

    printf("!!! ERROR: Arena sin espacio !!!\n");
    return NULL;
}

void arena_reset(Arena *a)
{
    a->usado = 0;
    printf("Arena reseteada (Puntero devuelto a 0).\n");
}