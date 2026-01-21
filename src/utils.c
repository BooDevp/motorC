#include <SDL3/SDL.h>
#include <ctype.h>
#include "modelo.h"

void calcular_frames(float *fps_actuales, Uint64 *frames_contados, char *texto_fps, int tam_buffer, Uint64 *tiempo_anterior)
{
    (*frames_contados)++;

    Uint64 tiempo_actual = SDL_GetTicks();
    Uint64 diferencia = tiempo_actual - *tiempo_anterior;

    if (diferencia >= 1000)
    {
        *fps_actuales = *frames_contados / (diferencia / 1000.0f);
        *tiempo_anterior = tiempo_actual;
        *frames_contados = 0;
        SDL_snprintf(texto_fps, tam_buffer, "FPS: %.2f", *fps_actuales);
    }
}

// Función auxiliar para saltar espacios en blanco (como lo hacen los parsers reales)
char *trim_vacio(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    return s;
}

// Info
void imprimir_info_modelo(Modelo *f)
{
    if (f == NULL)
    {
        printf("--- Info Modelo: NULL ---\n");
        return;
    }

    printf("--- Info Modelo ---\n");
    printf("Direccion en RAM: %p\n", (void *)f);
    printf("Vertices: %d (En RAM: %p)\n", f->n_puntos, (void *)f->vertices);
    printf("Aristas:  %d (En RAM: %p)\n", f->n_aristas, (void *)f->aristas);
    printf("Centro:   (%.2f, %.2f, %.2f)\n", f->cx, f->cy, f->cz);

    // Si quieres ver los primeros 3 vértices (X, Y, Z)
    if (f->n_puntos > 0)
    {
        printf("Primer vertice: X:%.2f Y:%.2f Z:%.2f\n",
               f->vertices[0], f->vertices[1], f->vertices[2]);
    }
    printf("-------------------\n");
}
