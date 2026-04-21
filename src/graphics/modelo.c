#define SDL_MAIN_HANDLED
#include "graphics/modelo.h"
#include <math.h>
#include "config/modelos_id.h"

void normalizacion_modelo_centrar(Modelo *f)
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