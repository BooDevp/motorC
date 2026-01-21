#include "modelo.h"
#include <ctype.h>

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
    printf("Caras: %d (En RAM: %p)\n", f->n_caras, (void *)f->caras);
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
    f->n_caras = 0; // Ahora contamos caras (triángulos)

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
            int vertices_en_linea = 0;
            char *ptr = linea + 1;
            while (*ptr)
            {
                if (isspace(*ptr) && !isspace(*(ptr + 1)) && *(ptr + 1) != '\0')
                    vertices_en_linea++;
                ptr++;
            }
            // TRIANGULACIÓN: Una cara de N vértices produce (N - 2) triángulos
            // Ejemplo: Cuadrado (4 vértices) -> 4 - 2 = 2 triángulos.
            if (vertices_en_linea >= 3)
            {
                f->n_caras += (vertices_en_linea - 2);
            }
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

            // Convertimos el polígono en triángulos
            // Usamos la técnica de "abanico" (Triangle Fan)
            for (int i = 1; i < count - 1; i++)
            {
                f->caras[c_ptr].v1 = v_indices[0];
                f->caras[c_ptr].v2 = v_indices[i];
                f->caras[c_ptr].v3 = v_indices[i + 1];
                c_ptr++;
            }
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
        // char *ruta = "./assets/models/Handpopus.obj";
        char *ruta = "./assets/models/Cubo.obj";

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
    if (f == NULL || f->vertices == NULL || f->caras == NULL) return;

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    for (int i = 0; i < f->n_caras; i++)
    {
        int idx[3] = { f->caras[i].v1, f->caras[i].v2, f->caras[i].v3 };
        float px[3], py[3]; // , pz[3]; // Guardamos también la Z proyectada

        for (int j = 0; j < 3; j++)
        {
            float vx = f->vertices[idx[j] * 3];
            float vy = f->vertices[idx[j] * 3 + 1];
            float vz = f->vertices[idx[j] * 3 + 2];

            vx -= f->cx; vy -= f->cy; vz -= f->cz;
            rotar_punto(&vx, &vy, &vz, angulo, 'y');
            vz += distancia_camara;

            // Guardamos las coordenadas proyectadas
            proyectar_a_pixel(vx, vy, vz, escala, escala, &px[j], &py[j], ventana_ancho, ventana_alto);
           //  pz[j] = vz; // Guardamos la profundidad
        }

        // --- BACKFACE CULLING ---
        // Calculamos los vectores de dos lados del triángulo en pantalla
        float x1 = px[1] - px[0];
        float y1 = py[1] - py[0];
        float x2 = px[2] - px[0];
        float y2 = py[2] - py[0];

        // El valor "cross" nos dice la orientación (sentido horario o antihorario)
        // En 2D, esto equivale a la dirección de la normal respecto a la cámara
        float cross_product = (x1 * y2) - (y1 * x2);

        // Si el producto es menor que 0, la cara está mirando hacia atrás.
        // ¡No la dibujamos y saltamos a la siguiente cara!
        if (cross_product < 0) {
            continue; 
        }

        // --- DIBUJO DE LÍNEAS (Solo si pasó la prueba anterior) ---
        SDL_RenderLine(renderer, px[0], py[0], px[1], py[1]);
        SDL_RenderLine(renderer, px[1], py[1], px[2], py[2]);
        SDL_RenderLine(renderer, px[2], py[2], px[0], py[0]);
    }
}