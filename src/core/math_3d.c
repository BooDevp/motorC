#include "core/math_3d.h"

void rotar_x(float *y, float *z, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float y_nueva = (*y * c) - (*z * s);
    float z_nueva = (*y * s) + (*z * c);
    *y = y_nueva;
    *z = z_nueva;
}

void rotar_y(float *x, float *z, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float x_nueva = (*x * c) - (*z * s);
    float z_nueva = (*x * s) + (*z * c);
    *x = x_nueva;
    *z = z_nueva;
}

void rotar_z(float *x, float *y, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float x_nueva = (*x * c) - (*y * s);
    float y_nueva = (*x * s) + (*y * c);
    *x = x_nueva;
    *y = y_nueva;
}

void proyectar_a_pixel(float x, float y, float z, float escala_x, float escala_y, float *px, float *py, int area_ancho, int area_alto)
{
    if (z == 0) z = 0.001f; // Evitar división por cero
    float x_2d = x / z;
    float y_2d = y / z;
    
    *px = (x_2d * escala_x) + (area_ancho / 2.0f);
    *py = (-y_2d * escala_y) + (area_alto / 2.0f);
}

void calcular_centros(float *vertices, int total_puntos, float *cx, float *cy, float *cz)
{
    float min_x = vertices[0], max_x = vertices[0];
    float min_y = vertices[1], max_y = vertices[1];
    float min_z = vertices[2], max_z = vertices[2];
    for (int i = 0; i < total_puntos; i++)
    {
        if (vertices[i * 3] < min_x)
            min_x = vertices[i * 3];
        if (vertices[i * 3] > max_x)
            max_x = vertices[i * 3];
        if (vertices[i * 3 + 1] < min_y)
            min_y = vertices[i * 3 + 1];
        if (vertices[i * 3 + 1] > max_y)
            max_y = vertices[i * 3 + 1];
        if (vertices[i * 3 + 2] < min_z)
            min_z = vertices[i * 3 + 2];
        if (vertices[i * 3 + 2] > max_z)
            max_z = vertices[i * 3 + 2];
    }
    *cx = (min_x + max_x) / 2.0f;
    *cy = (min_y + max_y) / 2.0f;
    *cz = (min_z + max_z) / 2.0f;
}