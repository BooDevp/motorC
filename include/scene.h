#ifndef SCENE_H
#define SCENE_H

#include "modelo.h"
#include "gestion_memoria.h"
#include "math_3d.h"

typedef struct
{
    int modelo_index;
    Modelo *modelo;
    Vec3 posicion;
    Vec3 rotacion;
    Vec3 vel_rotacion;
    float escala;
    Uint8 color_r, color_g, color_b;
    int transparencia;
} Instancia;

typedef struct
{
    Instancia *instancias;
    int n_instancias;
} Escena;

static Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales)
{
    arena_reset(arena); // Limpiar memoria de la escena anterior

    Escena *escena = (Escena *)arena_push(arena, sizeof(Escena));
    escena->n_instancias = num;
    escena->instancias = (Instancia *)arena_push(arena, sizeof(Instancia) * num);

    for (int i = 0; i < num; i++)
    {        
        escena->instancias[i] = config[i];        
        int idx = config[i].modelo_index;
        escena->instancias[i].modelo = modelos_globales[idx];
    }

    arena_reporte(arena, "ESCENA UNIFICADA CARGADA");
    return escena;
}

static void actualizar_escena(Escena *escena, float dt)
{
    if (!escena)
        return;

    for (int i = 0; i < escena->n_instancias; i++)
    {
        Instancia *inst = &escena->instancias[i];
        inst->rotacion.x += inst->vel_rotacion.x * dt;
        inst->rotacion.y += inst->vel_rotacion.y * dt;
        inst->rotacion.z += inst->vel_rotacion.z * dt;
    }
}

static void pintar_instancia(Instancia *inst, SDL_Renderer *renderer, float distancia_camara, int ventana_ancho, int ventana_alto, float escala_global)
{
    if (!inst || !inst->modelo)
        return;

    Modelo *f = inst->modelo;

    SDL_SetRenderDrawColor(renderer, inst->color_r, inst->color_g, inst->color_b, inst->transparencia);

    for (int i = 0; i < f->n_caras; i++)
    {
        Cara cara = f->caras[i];
        float px[64], py[64];

        for (int j = 0; j < cara.n_vertices; j++)
        {
            int idx = cara.vertices[j];
            float vx = f->vertices[idx * 3];
            float vy = f->vertices[idx * 3 + 1];
            float vz = f->vertices[idx * 3 + 2];

            // Escala
            vx *= inst->escala;
            vy *= inst->escala;
            vz *= inst->escala;

            // Rotación propia
            rotar_x(&vy, &vz, inst->rotacion.x);
            rotar_y(&vx, &vz, inst->rotacion.y);
            rotar_z(&vx, &vy, inst->rotacion.z);

            // Posición en el mundo
            vx += inst->posicion.x;
            vy += inst->posicion.y;
            vz += inst->posicion.z;

            // Cámara y Proyección
            vz += distancia_camara;
            proyectar_a_pixel(vx, vy, vz, escala_global, escala_global, &px[j], &py[j], ventana_ancho, ventana_alto);
        }

        for (int j = 0; j < cara.n_vertices; j++)
        {
            int next = (j + 1) % cara.n_vertices;
            SDL_RenderLine(renderer, (int)px[j], (int)py[j], (int)px[next], (int)py[next]);
        }
    }
}

static void pintar_escena(Escena *escena, SDL_Renderer *renderer, float distancia_camara, int ventana_ancho, int ventana_alto, float escala_global)
{
    if (!escena)
        return;

    for (int i = 0; i < escena->n_instancias; i++)
    {
        pintar_instancia(&escena->instancias[i], renderer, distancia_camara, ventana_ancho, ventana_alto, escala_global);
    }
}

#endif