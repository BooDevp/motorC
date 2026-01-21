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
        char *ruta = "./assets/models/Handpopus.obj";
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

// Pequeña utilidad para intercambiar valores (swap)
static void swap_float(float *a, float *b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

static void dibujar_triangulo_relleno_z(SDL_Renderer *renderer, float *z_buffer, int ancho_v, int alto_v,
                                        float x0, float y0, float z0,
                                        float x1, float y1, float z1,
                                        float x2, float y2, float z2)
{
    // 1. Ordenar por Y (y0 <= y1 <= y2)
    if (y0 > y1)
    {
        swap_float(&x0, &x1);
        swap_float(&y0, &y1);
        swap_float(&z0, &z1);
    }
    if (y0 > y2)
    {
        swap_float(&x0, &x2);
        swap_float(&y0, &y2);
        swap_float(&z0, &z2);
    }
    if (y1 > y2)
    {
        swap_float(&x1, &x2);
        swap_float(&y1, &y2);
        swap_float(&z1, &z2);
    }

    if (y2 - y0 < 0.0001f)
        return;

    // 2. Pendientes inversas (X y Z)
    float dx02 = (x2 - x0) / (y2 - y0);
    float dz02 = (z2 - z0) / (y2 - y0);

    float dx01 = (y1 - y0 > 0.0001f) ? (x1 - x0) / (y1 - y0) : 0;
    float dz01 = (y1 - y0 > 0.0001f) ? (z1 - z0) / (y1 - y0) : 0;

    float dx12 = (y2 - y1 > 0.0001f) ? (x2 - x1) / (y2 - y1) : 0;
    float dz12 = (y2 - y1 > 0.0001f) ? (z2 - z1) / (y2 - y1) : 0;

    // 3. Scanline de arriba a abajo
    for (int y = (int)ceilf(y0); y <= (int)floorf(y2); y++)
    {
        float xa = x0 + (y - y0) * dx02;
        float za = z0 + (y - y0) * dz02;
        float xb, zb;

        if (y < y1)
        {
            xb = x0 + (y - y0) * dx01;
            zb = z0 + (y - y0) * dz01;
        }
        else
        {
            xb = x1 + (y - y1) * dx12;
            zb = z1 + (y - y1) * dz12;
        }

        if (xa > xb)
        {
            swap_float(&xa, &xb);
            swap_float(&za, &zb);
        }

        int ix_inicio = (int)roundf(xa);
        int ix_fin = (int)roundf(xb);

        for (int x = ix_inicio; x <= ix_fin; x++)
        {
            if (x >= 0 && x < ancho_v && y >= 0 && y < alto_v)
            {
                // Interpolamos Z horizontalmente
                float t = (ix_inicio == ix_fin) ? 0 : (float)(x - ix_inicio) / (ix_fin - ix_inicio);
                float z_pixel = za + t * (zb - za);

                int indice = y * ancho_v + x;
                // PRUEBA DE PROFUNDIDAD
                if (z_pixel < z_buffer[indice])
                {
                    z_buffer[indice] = z_pixel;
                    SDL_RenderPoint(renderer, (float)x, (float)y);
                }
            }
        }
    }
}

void pintar_modelo(Modelo *f, SDL_Renderer *renderer, float *z_buffer, float angulo, float distancia_camara, int ventana_ancho, int ventana_alto, float escala)
{
    if (f == NULL || z_buffer == NULL)
        return;

    float luz[3] = {0.0f, 0.0f, -1.0f};

    for (int i = 0; i < f->n_caras; i++)
    {
        int idx[3] = {f->caras[i].v1, f->caras[i].v2, f->caras[i].v3};
        float v_rotado[3][3], px[3], py[3];

        for (int j = 0; j < 3; j++)
        {
            float vx = f->vertices[idx[j] * 3];
            float vy = f->vertices[idx[j] * 3 + 1];
            float vz = f->vertices[idx[j] * 3 + 2];

            rotar_punto(&vx, &vy, &vz, angulo, 'y');
            vz += distancia_camara;

            v_rotado[j][0] = vx;
            v_rotado[j][1] = vy;
            v_rotado[j][2] = vz;
            proyectar_a_pixel(vx, vy, vz, escala, escala, &px[j], &py[j], ventana_ancho, ventana_alto);
        }

        float cross = (px[1] - px[0]) * (py[2] - py[0]) - (py[1] - py[0]) * (px[2] - px[0]);
        if (cross > 0)
        {
            float normal[3];
            calcular_normal(v_rotado[0], v_rotado[1], v_rotado[2], normal);
            float intensidad = calcular_iluminacion(normal, luz);
            int color_v = (int)((intensidad * 0.9f + 0.1f) * 255);

            SDL_SetRenderDrawColor(renderer, 0, color_v, 0, 255);

            // Llamamos a la versión con Z-Buffer
            dibujar_triangulo_relleno_z(renderer, z_buffer, ventana_ancho, ventana_alto,
                                        px[0], py[0], v_rotado[0][2],
                                        px[1], py[1], v_rotado[1][2],
                                        px[2], py[2], v_rotado[2][2]);
        }
    }
}

float *inicializar_zbuffer(Arena *arena, int ancho, int alto)
{
    // Pedimos memoria a nuestra Arena
    float *buffer = (float *)arena_push(arena, ancho * alto * sizeof(float));
    if (buffer)
    {
        limpiar_zbuffer(buffer, ancho, alto);
    }
    return buffer;
}

void limpiar_zbuffer(float *z_buffer, int ancho, int alto)
{
    if (!z_buffer)
        return;
    // Llenamos el buffer con la distancia máxima (el "fondo" infinito)
    for (int i = 0; i < ancho * alto; i++)
    {
        z_buffer[i] = FLT_MAX;
    }
}