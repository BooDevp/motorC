#ifndef CAMARA_H
#define CAMARA_H

#include <math.h>

#define TO_RAD (SDL_PI_F / 180.0f)

typedef enum
{
    CAMARA_LIBRE,
    CAMARA_ISOMETRICA,
    CAMARA_FRONTAL
} TipoCamara;

typedef struct
{
    TipoCamara tipo;
    float posicion[3];
    float rotacion[3];
    float distancia;

    // Propiedades de lente/proyección
    float fov;
    float near_plane;
    float far_plane;
} Camara;

// Inicialización rápida con valores seguros
static inline void crear_camara_defecto(Camara *cam)
{
    cam->tipo = CAMARA_FRONTAL;
    cam->distancia = 4.0f;
    cam->fov = 45.0f;
    cam->near_plane = 0.1f;
    cam->far_plane = 100.0f;
}

static inline void calcular_matriz_vista(Camara *cam, float *view_matrix)
{
    float rx = (cam->tipo == CAMARA_ISOMETRICA) ? 35.264f : cam->rotacion[0];
    float ry = (cam->tipo == CAMARA_ISOMETRICA) ? 45.0f : cam->rotacion[1];

    float radX = rx * TO_RAD;
    float radY = ry * TO_RAD;
    float cx = cosf(radX), sx = sinf(radX);
    float cy = cosf(radY), sy = sinf(radY);

    view_matrix[0] = cy;
    view_matrix[4] = 0.0f;
    view_matrix[8] = sy;
    view_matrix[12] = -cam->posicion[0];
    view_matrix[1] = sx * sy;
    view_matrix[5] = cx;
    view_matrix[9] = -sx * cy;
    view_matrix[13] = -cam->posicion[1];
    view_matrix[2] = -cx * sy;
    view_matrix[6] = sx;
    view_matrix[10] = cx * cy;
    view_matrix[14] = -cam->distancia;
    view_matrix[3] = 0.0f;
    view_matrix[7] = 0.0f;
    view_matrix[11] = 0.0f;
    view_matrix[15] = 1.0f;
}

static inline void calcular_matriz_proyeccion(Camara *cam, int width, int height, float *proj_matrix)
{
    float aspect = (float)width / (float)height;
    float fov_rad = cam->fov * TO_RAD;
    float f = 1.0f / tanf(fov_rad / 2.0f);

    // Limpiar matriz
    for (int i = 0; i < 16; i++)
        proj_matrix[i] = 0;

    proj_matrix[0] = f / aspect;
    proj_matrix[5] = f;
    proj_matrix[10] = (cam->far_plane + cam->near_plane) / (cam->near_plane - cam->far_plane);
    proj_matrix[11] = -1.0f;
    proj_matrix[14] = (2.0f * cam->far_plane * cam->near_plane) / (cam->near_plane - cam->far_plane);
}

#endif