#include "graphics/escena.h"

Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales)
{
    arena_reset(arena);

    Escena *escena = (Escena *)arena_push(arena, sizeof(Escena));
    escena->n_instancias = num;
    escena->instancias = (Instancia *)arena_push(arena, sizeof(Instancia) * num);
    escena->catalogo_referencia = modelos_globales;

    for (int i = 0; i < num; i++)
    {
        escena->instancias[i] = config[i];
        escena->instancias[i].modelo = modelos_globales[config[i].modelo_index];
    }

    arena_reporte(arena, "ESCENA UNIFICADA CARGADA");
    return escena;
}

void actualizar_escena(Escena *escena, float dt)
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

static void pintar_instancia(Instancia *inst, SDL_Renderer *renderer, float distancia_camara, Layout *layout)
{
    if (!inst || !inst->modelo)
        return;

    Modelo *f = inst->modelo;

    SDL_SetRenderDrawColor(renderer, inst->color_r, inst->color_g, inst->color_b, inst->transparencia);

    for (int i = 0; i < f->n_caras; i++)
    {
        Cara cara = f->caras[i];
        float px[64], py[64];
        bool cara_fuera = false;

        for (int j = 0; j < cara.n_vertices; j++)
        {
            int idx = cara.vertices[j];

            // ESCALADO LOCAL DEL MODELO
            float vx = f->vertices[idx * 3] * inst->escala;
            float vy = f->vertices[idx * 3 + 1] * inst->escala;
            float vz = f->vertices[idx * 3 + 2] * inst->escala;

            // ROTACIÓN
            rotar_x(&vy, &vz, inst->rotacion.x);
            rotar_y(&vx, &vz, inst->rotacion.y);
            rotar_z(&vx, &vy, inst->rotacion.z);

            // TRASLACIÓN
            vx += inst->posicion.x;
            vy += inst->posicion.y;
            vz += inst->posicion.z + distancia_camara;

            if (vz < 0.1f)
            {
                cara_fuera = true;
                break;
            }

            proyectar_a_pixel(vx, vy, vz, layout->escala_proyeccion, layout->escala_proyeccion, &px[j], &py[j], layout->juego_w, layout->juego_h);
        }

        if (!cara_fuera)
        {
            for (int j = 0; j < cara.n_vertices; j++)
            {
                int next = (j + 1) % cara.n_vertices;                
                SDL_RenderLine(renderer, px[j], py[j], px[next], py[next]);
            }
        }
    }
}

void pintar_escena(Escena *escena, SDL_Renderer *renderer, float dist, Layout *layout)
{
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.fondo.r, TEMA_DEFAULT.fondo.g, TEMA_DEFAULT.fondo.b, 255);
    SDL_RenderClear(renderer);

    if (!escena || !renderer)
        return;

    // Me adueño del viewport para dibujar la escena.
    SDL_SetRenderViewport(renderer, &layout->viewport_juego);

    // Fondo del juego (dentro del viewport)
    SDL_FRect fondo = {0.0f, 0.0f, (float)layout->juego_w, (float)layout->juego_h};
    SDL_SetRenderDrawColor(renderer, TEMA_DEFAULT.fondo.r, TEMA_DEFAULT.fondo.g, TEMA_DEFAULT.fondo.b, 255);
    SDL_RenderFillRect(renderer, &fondo);

    for (int i = 0; i < escena->n_instancias; i++)
    {
        pintar_instancia(&escena->instancias[i], renderer, dist, layout);
    }

    // Dejo el viewport global para el layout y la barra de estado
    SDL_SetRenderViewport(renderer, NULL);
}