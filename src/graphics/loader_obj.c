#define SDL_MAIN_HANDLED

#include "graphics/loader_obj.h"

static Modelo *get_modelo_obj(Arena *arena, const char *ruta)
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
    normalizacion_modelo_centrar(f);
    return f;
}

Modelo **inicializar_catalogo_modelos(Arena *arena)
{
    Modelo **catalogo = (Modelo **)arena_push(arena, sizeof(Modelo *) * TOTAL_MODELOS);

    for (int i = 0; i < TOTAL_MODELOS; i++)
    {
        catalogo[i] = get_modelo_obj(arena, rutas_modelos_globales[i]);
        if (catalogo[i])
        {
            calcular_centros(catalogo[i]->vertices, catalogo[i]->n_puntos,
                             &catalogo[i]->cx, &catalogo[i]->cy, &catalogo[i]->cz);
        }
    }
    return catalogo;
}