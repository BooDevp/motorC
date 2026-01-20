#include "modelo.h"
#include <ctype.h>

Modelo *get_modelo_obj(Arena *arena, const char *ruta)
{
    FILE *archivo = fopen(ruta, "r");
    if (!archivo)
    {
        printf("Error: No se pudo abrir %s\n", ruta);
        return NULL;
    }

    Modelo *f = (Modelo *)arena_push(arena, sizeof(Modelo));
    if (!f)
        return NULL;
    f->n_puntos = 0;
    f->n_aristas = 0;

    char linea[512]; // Buffer para las líneas

    // --- PRIMERA PASADA: CONTEO REAL ---
    while (fgets(linea, sizeof(linea), archivo))
    {
        char *ptr = trim_vacio(linea);
        if (ptr[0] == 'v' && isspace(ptr[1]))
        {
            f->n_puntos++;
        }
        else if ((ptr[0] == 'f' || ptr[0] == 'l') && isspace(ptr[1]))
        {
            int elementos_en_linea = 0;
            char *token = strtok(ptr + 1, " \t\r\n");
            while (token)
            {
                elementos_en_linea++;
                token = strtok(NULL, " \t\r\n");
            }
            // Si es 'f' (cara), son N aristas (bucle cerrado).
            // Si es 'l' (línea), son N-1 aristas.
            if (ptr[0] == 'f' && elementos_en_linea > 2)
                f->n_aristas += elementos_en_linea;
            if (ptr[0] == 'l' && elementos_en_linea > 1)
                f->n_aristas += (elementos_en_linea - 1);
        }
    }

    // 2. RESERVA EXACTA
    f->vertices = (float *)arena_push(arena, f->n_puntos * 3 * sizeof(float));
    f->aristas = (int *)arena_push(arena, f->n_aristas * 2 * sizeof(int));

    if (!f->vertices || !f->aristas)
    {
        fclose(archivo);
        return NULL;
    }

    // --- SEGUNDA PASADA: LECTURA SEGURA ---
    rewind(archivo);
    int v_ptr = 0;
    int a_ptr = 0;
    int total_aristas_reales = 0;

    while (fgets(linea, sizeof(linea), archivo))
    {
        char *ptr = trim_vacio(linea);
        char tipo = ptr[0];

        if (tipo == 'v' && isspace(ptr[1]))
        {
            sscanf(ptr, "v %f %f %f", &f->vertices[v_ptr], &f->vertices[v_ptr + 1], &f->vertices[v_ptr + 2]);
            v_ptr += 3;
        }
        else if ((tipo == 'f' || tipo == 'l') && isspace(ptr[1]))
        {
            int v_indices[64]; // Soportamos caras de hasta 64 vértices (blindado)
            int count = 0;
            char *token = strtok(ptr + 1, " \t\r\n");

            while (token && count < 64)
            {
                // Los archivos OBJ a veces tienen v/vt/vn, solo queremos la 'v' (el primer número)
                v_indices[count] = atoi(token) - 1;
                count++;
                token = strtok(NULL, " \t\r\n");
            }

            for (int i = 0; i < count; i++)
            {
                // Lógica de conexión:
                if (tipo == 'f')
                {
                    // Cara: conecta cada uno con el siguiente, y el último con el primero
                    f->aristas[a_ptr++] = v_indices[i];
                    f->aristas[a_ptr++] = v_indices[(i + 1) % count];
                    total_aristas_reales++;
                }
                else if (tipo == 'l' && i < count - 1)
                {
                    // Línea: conecta cada uno con el siguiente, pero no cierra el bucle
                    f->aristas[a_ptr++] = v_indices[i];
                    f->aristas[a_ptr++] = v_indices[i + 1];
                    total_aristas_reales++;
                }
            }
        }
    }

    // Ajuste final: La realidad manda sobre el conteo previo
    f->n_aristas = total_aristas_reales;

    // --- NORMALIZACIÓN MEJORADA (Efecto Zoom Uniforme) ---
    normalizacion_objeto_centrar(f);

    fclose(archivo);
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
        char *ruta = "./resources/cubo.obj";

        ruta = "./resources/cubo.obj";

        Modelo *nuevo = get_modelo_obj(arena, ruta);
        if (nuevo)
        {
            calcular_centros(nuevo->vertices, nuevo->n_puntos, &nuevo->cx, &nuevo->cy, &nuevo->cz);
            *modelo_actual = nuevo;
        }

        *ultimo_clic = tiempo_actual;
    }
}

void normalizacion_objeto_centrar(Modelo *f)
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
    printf("Aristas:  %d (En RAM: %p)\n", f->n_aristas, (void *)f->aristas);
    printf("Centro:   (%.2f, %.2f, %.2f)\n", f->cx, f->cy, f->cz);

    // Si quieres ver los primeros 3 vértices (X, Y, Z)
    if (f->n_puntos > 0)
    {
        printf("Primer vertice: X:%.2f Y:%.2f Z:%.2f\n",
               f->vertices[0], f->vertices[1], f->vertices[2]);
    }
    printf("-------------------\n");
}
