#define SDL_MAIN_HANDLED
#include "utils.h"

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