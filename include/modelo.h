#ifndef MODELO_H
#define MODELO_H

#include "gestion_memoria.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include "math_3d.h"
#include <ctype.h>
#include <float.h>

typedef struct
{
    int *vertices;
    int n_vertices;
} Cara;

typedef struct
{
    float *vertices;
    int n_puntos;
    Cara *caras;
    int n_caras;
    float cx, cy, cz;
} Modelo;

static void normalizacion_objeto_centrar(Modelo *f)
{
    if (f->n_puntos > 0)
    {
        float min_x = f->vertices[0], max_x = f->vertices[0];
        float min_y = f->vertices[1], max_y = f->vertices[1];
        float min_z = f->vertices[2], max_z = f->vertices[2];

        // Encontrar límites
        for (int i = 0; i < f->n_puntos; i++)
        {
            if (f->vertices[i * 3] < min_x)
                min_x = f->vertices[i * 3];
            if (f->vertices[i * 3] > max_x)
                max_x = f->vertices[i * 3];
            if (f->vertices[i * 3 + 1] < min_y)
                min_y = f->vertices[i * 3 + 1];
            if (f->vertices[i * 3 + 1] > max_y)
                max_y = f->vertices[i * 3 + 1];
            if (f->vertices[i * 3 + 2] < min_z)
                min_z = f->vertices[i * 3 + 2];
            if (f->vertices[i * 3 + 2] > max_z)
                max_z = f->vertices[i * 3 + 2];
        }

        // Centrar el objeto en (0,0,0)
        float cx = (min_x + max_x) / 2.0f;
        float cy = (min_y + max_y) / 2.0f;
        float cz = (min_z + max_z) / 2.0f;

        float max_dist_sq = 0;
        for (int i = 0; i < f->n_puntos; i++)
        {
            f->vertices[i * 3] -= cx;
            f->vertices[i * 3 + 1] -= cy;
            f->vertices[i * 3 + 2] -= cz;

            // Buscar el punto más lejano al nuevo centro (0,0,0)
            float d_sq = f->vertices[i * 3] * f->vertices[i * 3] +
                         f->vertices[i * 3 + 1] * f->vertices[i * 3 + 1] +
                         f->vertices[i * 3 + 2] * f->vertices[i * 3 + 2];
            if (d_sq > max_dist_sq)
                max_dist_sq = d_sq;
        }
        
        float max_dist = sqrtf(max_dist_sq);
        if (max_dist > 0)
        {
            float factor = 0.5f / max_dist;
            for (int i = 0; i < f->n_puntos * 3; i++)
            {
                f->vertices[i] *= factor;
            }
        }
    }
}

// Cargar modelo desde archivo .obj (Blender)
Modelo *get_modelo_obj(Arena *arena, const char *ruta)
{
    FILE *archivo = fopen(ruta, "r");
    if (!archivo)
        return NULL;

    Modelo *f = (Modelo *)arena_push(arena, sizeof(Modelo));
    f->n_puntos = 0;
    f->n_caras = 0;

    char linea[512];

    // --- CONTEO ---
    while (fgets(linea, sizeof(linea), archivo))
    {
        if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\r')
            continue;

        if (linea[0] == 'v' && isspace(linea[1]))
        {
            f->n_puntos++;
        }
        else if (linea[0] == 'f' && isspace(linea[1]))
        {
            f->n_caras++;
        }
    }

    // RESERVA DE MEMORIA EXACTA
    f->vertices = (float *)arena_push(arena, f->n_puntos * 3 * sizeof(float));
    f->caras = (Cara *)arena_push(arena, f->n_caras * sizeof(Cara));

    // --- LECTURA ---
    rewind(archivo);
    int v_ptr = 0;
    int c_ptr = 0;

    while (fgets(linea, sizeof(linea), archivo))
    {
        if (linea[0] == 'v' && isspace(linea[1]))
        {
            sscanf(linea, "v %f %f %f", &f->vertices[v_ptr], &f->vertices[v_ptr + 1], &f->vertices[v_ptr + 2]);
            v_ptr += 3;
        }
        else if (linea[0] == 'f' && isspace(linea[1]))
        {
            int v_indices[64];
            int count = 0;
            char *token = strtok(linea + 1, " \t\r\n");

            while (token && count < 64)
            {
                v_indices[count] = atoi(token) - 1;
                count++;
                token = strtok(NULL, " \t\r\n");
            }

            f->caras[c_ptr].n_vertices = count;
            f->caras[c_ptr].vertices = (int *)arena_push(arena, count * sizeof(int));
            for (int i = 0; i < count; i++)
            {
                f->caras[c_ptr].vertices[i] = v_indices[i];
            }
            c_ptr++;
        }
    }

    fclose(archivo);
    normalizacion_objeto_centrar(f);
    return f;
}

#endif