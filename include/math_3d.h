#ifndef MATH_3D_H
#define MATH_3D_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void proyectar_a_pixel(float x, float y, float z, float escala_x, float escala_y, float *px, float *py, int ventana_ancho, int ventana_alto);
void rotar_punto(float *x, float *y, float *z, float grados, char eje);
void calcular_centros(float *vertices, int total_puntos, float *cx, float *cy, float *cz);
void calcular_normal(float v0[3], float v1[3], float v2[3], float normal[3]);
float calcular_iluminacion(float normal[3], float direccion_luz[3]);

#endif