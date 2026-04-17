#ifndef MATH_3D_H
#define MATH_3D_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static void rotar_x(float *y, float *z, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float y_nueva = (*y * c) - (*z * s);
    float z_nueva = (*y * s) + (*z * c);
    *y = y_nueva;
    *z = z_nueva;
}

static void rotar_y(float *x, float *z, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float x_nueva = (*x * c) - (*z * s);
    float z_nueva = (*x * s) + (*z * c);
    *x = x_nueva;
    *z = z_nueva;
}

static void rotar_z(float *x, float *y, float grados)
{
    float rad = grados * (M_PI / 180.0f);
    float s = sinf(rad), c = cosf(rad);
    float x_nueva = (*x * c) - (*y * s);
    float y_nueva = (*x * s) + (*y * c);
    *x = x_nueva;
    *y = y_nueva;
}

static void proyectar_a_pixel(float x, float y, float z, float escala_x, float escala_y, float *px, float *py, int ventana_ancho, int ventana_alto)
{
    if (z == 0)
        z = 0.001f; // Evitar división por cero
    float x_2d = x / z;
    float y_2d = y / z;
    *px = (x_2d * escala_x) + (ventana_ancho / 2.0f);
    *py = (-y_2d * escala_y) + (ventana_alto / 2.0f);
}

static void rotar_punto(float *x, float *y, float *z, float grados, char eje)
{
    switch (eje)
    {
    case 'x':
    case 'X':
        rotar_x(y, z, grados);
        break;
    case 'y':
    case 'Y':
        rotar_y(x, z, grados);
        break;
    case 'z':
    case 'Z':
        rotar_z(x, y, grados);
        break;
    }
}

static void calcular_centros(float *vertices, int total_puntos, float *cx, float *cy, float *cz)
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

// Calcula el vector normal de un triángulo definido por 3 puntos
/*static void calcular_normal(float v0[3], float v1[3], float v2[3], float normal[3])
{
    float edge1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
    float edge2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};

    // Producto cruz para obtener la normal
    normal[0] = edge1[1] * edge2[2] - edge1[2] * edge2[1];
    normal[1] = edge1[2] * edge2[0] - edge1[0] * edge2[2];
    normal[2] = edge1[0] * edge2[1] - edge1[1] * edge2[0];

    // Normalizar el vector (que mida 1)
    float longitud = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (longitud > 0)
    {
        normal[0] /= longitud;
        normal[1] /= longitud;
        normal[2] /= longitud;
    }
}

// Calcula la intensidad de luz (0.0 a 1.0)
static float calcular_iluminacion(float normal[3], float direccion_luz[3])
{
    // Producto punto (Dot Product)
    // Si da 1.0 están alineados (luz frontal), si da <= 0 está en sombra
    float intensidad = normal[0] * direccion_luz[0] +
                       normal[1] * direccion_luz[1] +
                       normal[2] * direccion_luz[2];

    if (intensidad < 0)
        intensidad = 0;
    return intensidad;
}*/

#endif