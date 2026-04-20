#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>
#include <ctype.h>

void calcular_frames(float *fps_actuales, Uint64 *frames_contados, char *texto_fps, int tam_buffer, Uint64 *tiempo_anterior);

#endif