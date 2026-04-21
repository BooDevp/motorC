#define SDL_MAIN_HANDLED
#include "graphics/scene.h"

Escena *cargar_escena_desde_config(Arena *arena, Instancia *config, int num, Modelo **modelos_globales)
{
    arena_reset(arena); // Limpiar memoria de la escena anterior

    Escena *escena = (Escena *)arena_push(arena, sizeof(Escena));
    escena->n_instancias = num;
    escena->instancias = (Instancia *)arena_push(arena, sizeof(Instancia) * num);

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

static void pintar_instancia(Instancia *inst, SDL_Renderer *renderer, float distancia_camara, int area_w, int area_h, float escala_global, int offset_x, int offset_y)
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
            float vx = f->vertices[idx * 3] * inst->escala;
            float vy = f->vertices[idx * 3 + 1] * inst->escala;
            float vz = f->vertices[idx * 3 + 2] * inst->escala;

            rotar_x(&vy, &vz, inst->rotacion.x);
            rotar_y(&vx, &vz, inst->rotacion.y);
            rotar_z(&vx, &vy, inst->rotacion.z);

            vx += inst->posicion.x;
            vy += inst->posicion.y;
            vz += inst->posicion.z + distancia_camara;

            if (vz < 0.1f)
            {
                cara_fuera = true;
                break;
            }

            proyectar_a_pixel(vx, vy, vz, escala_global, escala_global, &px[j], &py[j], area_w, area_h, offset_x, offset_y);
        }

        if (!cara_fuera)
        {
            for (int j = 0; j < cara.n_vertices; j++)
            {
                int next = (j + 1) % cara.n_vertices;
                SDL_RenderLine(renderer, (int)px[j], (int)py[j], (int)px[next], (int)py[next]);
            }
        }
    }
}

void pintar_escena(Escena *escena, SDL_Renderer *renderer, float distancia_camara, Layout *layout)
{
    if (!escena)
        return;

    // Definimos el rectángulo de recorte (ints)
    SDL_Rect viewport_rect = {layout->juego_offset_x, layout->juego_offset_y, layout->juego_w, layout->juego_h};

    // Definimos el rectángulo de fondo (floats)
    SDL_FRect fondo_rect = {(float)layout->juego_offset_x, (float)layout->juego_offset_y, (float)layout->juego_w, (float)layout->juego_h};

    // Activamos el recorte
    SDL_SetRenderClipRect(renderer, &viewport_rect);

    // Pintamos el fondo de la zona de juego
    SDL_SetRenderDrawColor(renderer,
                           TEMA_DEFAULT.fondo.r,
                           TEMA_DEFAULT.fondo.g,
                           TEMA_DEFAULT.fondo.b,
                           TEMA_DEFAULT.fondo.a);
    SDL_RenderFillRect(renderer, &fondo_rect);

    // Pintamos las instancias
    for (int i = 0; i < escena->n_instancias; i++)
    {
        pintar_instancia(&escena->instancias[i], renderer, distancia_camara, layout->juego_w, layout->juego_h, layout->escala_juego, layout->juego_offset_x, layout->juego_offset_y);
    }

    // Desactivamos el recorte
    SDL_SetRenderClipRect(renderer, NULL);
}