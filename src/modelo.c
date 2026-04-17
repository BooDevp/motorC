#include "modelo.h"
#include <ctype.h>
#include <float.h> // Para FLT_MAX

// Info
void imprimir_info_modelo(Modelo *f)
{
    if (f == NULL)
    {
        printf("--- Info Modelo: NULL ---\n");
        return;
    }

    printf("--- Info Modelo ---\n");
    printf("Direccion en RAM: %p\n", (void *)f);
    printf("Vertices: %d (En RAM: %p)\n", f->n_puntos, (void *)f->vertices);
    printf("Caras: %d\n", f->n_caras);
    for (int i = 0; i < f->n_caras; i++) {
        printf("  Cara %d: %d vertices\n", i, f->caras[i].n_vertices);
    }
    printf("Centro:   (%.2f, %.2f, %.2f)\n", f->cx, f->cy, f->cz);

    // Si quieres ver los primeros 3 vértices (X, Y, Z)
    if (f->n_puntos > 0)
    {
        printf("Primer vertice: X:%.2f Y:%.2f Z:%.2f\n",
               f->vertices[0], f->vertices[1], f->vertices[2]);
    }
    printf("-------------------\n");
}

static void normalizacion_objeto_centrar(Modelo *f)
{
    if (f->n_puntos > 0)
    {
        float min_x = f->vertices[0], max_x = f->vertices[0];
        float min_y = f->vertices[1], max_y = f->vertices[1];
        float min_z = f->vertices[2], max_z = f->vertices[2];

        // 1. Encontrar límites
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

        // 2. Centrar primero el objeto en (0,0,0)
        float cx = (min_x + max_x) / 2.0f;
        float cy = (min_y + max_y) / 2.0f;
        float cz = (min_z + max_z) / 2.0f;

        float max_dist_sq = 0;
        for (int i = 0; i < f->n_puntos; i++)
        {
            f->vertices[i * 3] -= cx;
            f->vertices[i * 3 + 1] -= cy;
            f->vertices[i * 3 + 2] -= cz;

            // 3. Buscar el punto más lejano al nuevo centro (0,0,0)
            float d_sq = f->vertices[i * 3] * f->vertices[i * 3] +
                         f->vertices[i * 3 + 1] * f->vertices[i * 3 + 1] +
                         f->vertices[i * 3 + 2] * f->vertices[i * 3 + 2];
            if (d_sq > max_dist_sq)
                max_dist_sq = d_sq;
        }

        // 4. Escalar para que el radio máximo sea 0.5 (ocupa 1 unidad de pantalla)
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
    f->n_caras = 0; // Contamos caras como están (no trianguladas)

    char linea[512];

    // --- PRIMERA PASADA: CONTEO ---
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
            f->n_caras++; // Cada línea 'f' es una cara
        }
    }

    // RESERVA DE MEMORIA EXACTA
    f->vertices = (float *)arena_push(arena, f->n_puntos * 3 * sizeof(float));
    f->caras = (Cara *)arena_push(arena, f->n_caras * sizeof(Cara));

    // --- SEGUNDA PASADA: LECTURA ---
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

            // Almacenamos la cara como está
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
    arena_reporte(arena, "DESPUES DE CARGAR MODELO");
    return f;
}

void cargar_modelo(Modelo **modelo_actual, Uint64 *ultimo_clic, const Uint64 COOLDOWN_BOTON, Arena *arena)
{
    Uint64 tiempo_actual = SDL_GetTicks();

    if (tiempo_actual - *ultimo_clic > COOLDOWN_BOTON)
    {
        // Bloqueamos el renderizado poniendo el puntero a NULL
        *modelo_actual = NULL;

        // Limpiamos la arena
        arena_reset(arena);

        // Cargamos el nuevo modelo
        char *ruta = "./assets/models/cubo.obj";
        // char *ruta = "./assets/models/Cubo.obj";

        Modelo *nuevo = get_modelo_obj(arena, ruta);
        if (nuevo)
        {
            calcular_centros(nuevo->vertices, nuevo->n_puntos, &nuevo->cx, &nuevo->cy, &nuevo->cz);
            *modelo_actual = nuevo;
        }

        *ultimo_clic = tiempo_actual;
    }
}

void pintar_modelo(Modelo *f, SDL_Renderer *renderer, float angulo, float distancia_camara, int ventana_ancho, int ventana_alto, float escala)
{
    if (f == NULL)
        return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanco para wireframe

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

            rotar_punto(&vx, &vy, &vz, angulo, 'y');
            vz += distancia_camara;

            proyectar_a_pixel(vx, vy, vz, escala, escala, &px[j], &py[j], ventana_ancho, ventana_alto);
        }

        // Dibujar líneas entre vértices consecutivos
        for (int j = 0; j < cara.n_vertices; j++)
        {
            int next = (j + 1) % cara.n_vertices;
            SDL_RenderLine(renderer, (int)px[j], (int)py[j], (int)px[next], (int)py[next]);
        }
    }
}