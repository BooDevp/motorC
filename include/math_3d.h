#ifndef MATH_3D_H
#define MATH_3D_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

typedef struct
{
    float x, y, z;
} Vec3;

void rotar_x(float *y, float *z, float grados);
void rotar_y(float *x, float *z, float grados);
void rotar_z(float *x, float *y, float grados);
void proyectar_a_pixel(float x, float y, float z, float escala_x, float escala_y, float *px, float *py, int area_ancho, int area_alto, int offset_x, int offset_y);
void calcular_centros(float *vertices, int total_puntos, float *cx, float *cy, float *cz);

#endif