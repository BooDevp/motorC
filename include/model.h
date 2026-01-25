/*
*  model.h
*  Motor OpenGL 3.3 - Carga de modelos OBJ con materiales
*/

#ifndef MODEL_H
#define MODEL_H

// Estructura de vértice "Interleaved" (todo junto para la GPU)
typedef struct
{
    float x, y, z;       // Posición
    float nx, ny, nz;    // Normales
    float u, v;          // UVs
    float r, g, b;       // Color Difuso (Kd)
    float ksr, ksg, ksb; // Color Especular (Ks)
    float ns;            // Exponente de brillo (Ns)
} Vertice;

// Estructura para manejar el modelo en el motor
typedef struct
{
    GLuint vao;
    GLuint vbo;
    int num_vertices;
} Modelo;

/**
 * Centra el modelo en el origen (0,0,0) y lo escala a un tamaño estándar.
 */
static void centrar_modelo(Vertice *vertices, unsigned int num_vertices)
{
    if (num_vertices == 0)
        return;

    // 1. Encontrar los límites (Bounding Box)
    float min_x = 1e10, max_x = -1e10;
    float min_y = 1e10, max_y = -1e10;
    float min_z = 1e10, max_z = -1e10;

    for (unsigned int i = 0; i < num_vertices; i++)
    {
        if (vertices[i].x < min_x)
            min_x = vertices[i].x;
        if (vertices[i].x > max_x)
            max_x = vertices[i].x;
        if (vertices[i].y < min_y)
            min_y = vertices[i].y;
        if (vertices[i].y > max_y)
            max_y = vertices[i].y;
        if (vertices[i].z < min_z)
            min_z = vertices[i].z;
        if (vertices[i].z > max_z)
            max_z = vertices[i].z;
    }

    // 2. Calcular el centro y la dimensión mayor
    float centro_x = (min_x + max_x) / 2.0f;
    float centro_y = (min_y + max_y) / 2.0f;
    float centro_z = (min_z + max_z) / 2.0f;

    float size_x = max_x - min_x;
    float size_y = max_y - min_y;
    float size_z = max_z - min_z;

    float max_dim = size_x;
    if (size_y > max_dim)
        max_dim = size_y;
    if (size_z > max_dim)
        max_dim = size_z;

    // Queremos que el objeto encaje en un cubo de tamaño 2 (-1 a 1)
    float factor_escala = (max_dim > 0) ? (2.0f / max_dim) : 1.0f;

    // 3. Aplicar transformación a cada vértice
    for (unsigned int i = 0; i < num_vertices; i++)
    {
        vertices[i].x = (vertices[i].x - centro_x) * factor_escala;
        vertices[i].y = (vertices[i].y - centro_y) * factor_escala;
        vertices[i].z = (vertices[i].z - centro_z) * factor_escala;
    }

    debug_log("Modelo centrado. Centro original: (%.2f, %.2f, %.2f), Escala: %.2f",
              centro_x, centro_y, centro_z, factor_escala);
}

/**
 * Cargar modelo desde un archivo OBJ
 * Modificado para manejar correctamente normales, UVs y prevenir corrupción de memoria.
 */
static inline bool cargar_modelo(Modelo *out_modelo, Arena *mi_arena, const char *ruta)
{
    debug_log("Cargando modelo con Materiales Completos: %s", ruta);

    fastObjMesh *mesh = fast_obj_read(ruta);
    if (!mesh)
        return false;

    unsigned int total_render_vertices = 0;
    for (unsigned int f = 0; f < mesh->face_count; f++)
    {
        total_render_vertices += (mesh->face_vertices[f] - 2) * 3;
    }

    size_t bytes_necesarios = sizeof(Vertice) * total_render_vertices;
    Vertice *datos_gpu = (Vertice *)arena_push(mi_arena, bytes_necesarios);
    if (!datos_gpu)
    {
        fast_obj_destroy(mesh);
        return false;
    }

    unsigned int curr_v = 0;
    unsigned int vert_offset = 0;

    for (unsigned int f = 0; f < mesh->face_count; f++)
    {
        unsigned int vertices_en_esta_cara = mesh->face_vertices[f];

        // --- EXTRAER TODO DEL MTL ---
        fastObjMaterial mat = mesh->materials[mesh->face_materials[f]];

        // Color Difuso (Kd) - QUITA EL IF DE LOS CEROS
        float dr = mat.Kd[0];
        float dg = mat.Kd[1];
        float db = mat.Kd[2];

        // Color Especular (Ks) y Brillo (Ns)
        float sr = mat.Ks[0];
        float sg = mat.Ks[1];
        float sb = mat.Ks[2];
        float shininess = mat.Ns;

        for (unsigned int v = 1; v < vertices_en_esta_cara - 1; v++)
        {
            unsigned int face_indices[3] = {0, v, v + 1};
            for (int i = 0; i < 3; i++)
            {
                fastObjIndex idx = mesh->indices[vert_offset + face_indices[i]];

                datos_gpu[curr_v].x = mesh->positions[idx.p * 3 + 0];
                datos_gpu[curr_v].y = mesh->positions[idx.p * 3 + 1];
                datos_gpu[curr_v].z = mesh->positions[idx.p * 3 + 2];

                if (mesh->normal_count > 1)
                {
                    datos_gpu[curr_v].nx = mesh->normals[idx.n * 3 + 0];
                    datos_gpu[curr_v].ny = mesh->normals[idx.n * 3 + 1];
                    datos_gpu[curr_v].nz = mesh->normals[idx.n * 3 + 2];
                }

                // Asignar datos de material al vértice
                datos_gpu[curr_v].r = dr;
                datos_gpu[curr_v].g = dg;
                datos_gpu[curr_v].b = db;
                datos_gpu[curr_v].ksr = sr;
                datos_gpu[curr_v].ksg = sg;
                datos_gpu[curr_v].ksb = sb;
                datos_gpu[curr_v].ns = shininess;

                curr_v++;
            }
        }
        vert_offset += vertices_en_esta_cara;
    }

    centrar_modelo(datos_gpu, total_render_vertices);

    // 3. Configuración de la GPU
    glGenVertexArrays(1, &out_modelo->vao);
    glGenBuffers(1, &out_modelo->vbo);
    glBindVertexArray(out_modelo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_modelo->vbo);
    glBufferData(GL_ARRAY_BUFFER, bytes_necesarios, datos_gpu, GL_STATIC_DRAW);

    // Layout de Atributos:
    // Pos(3), Norm(3), UV(2), Color(3), Spec(3), Shininess(1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Location 3: Color Difuso (Kd)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Location 4: Color Especular (Ks)
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // Location 5: Shininess (Ns)
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);

    out_modelo->num_vertices = total_render_vertices;
    glBindVertexArray(0);
    fast_obj_destroy(mesh);
    return true;
}

/**
 * Crea una matriz de rotación sobre el eje Y
 * Didáctico: Explica cómo se construye la matriz
 */
static void calculate_rotation_model_matrix_y(float angle_deg, float *matrix)
{
    // Convertir grados a radianes
    // 360 grados = 2 * PI radianes -> 1 grado = PI / 180 radianes
    float radians = angle_deg * (3.14159265f / 180.0f);

    // Calcular seno y coseno una sola vez
    float c = cosf(radians);
    float s = sinf(radians);

    // Construcción de la Matriz de Rotación Y (Column-Major para OpenGL)
    // |  cos(θ)   0   sin(θ)   0  |
    // |    0      1     0      0  |
    // | -sin(θ)   0   cos(θ)   0  |
    // |    0      0     0      1  |

    // Índice: Columna + Fila * 4 (si es 1D array representando matriz 4x4)
    // Pero aquí inicializamos manual:

    // Columna 1
    matrix[0] = c;
    matrix[1] = 0.0f;
    matrix[2] = -s;
    matrix[3] = 0.0f;

    // Columna 2
    matrix[4] = 0.0f;
    matrix[5] = 1.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    // Columna 3
    matrix[8] = s;
    matrix[9] = 0.0f;
    matrix[10] = c;
    matrix[11] = 0.0f;

    // Columna 4 (Posición)
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

/**
 * Configura las matrices (MVP)
 */
static inline void setup_matrices(GraphicsState *gs, int width, int height, float rotation_angle, AppState app)
{
    glUseProgram(gs->program);

    // Matriz de proyección (perspectiva)
    float aspect = (float)width / (float)height;
    float fov_rad = app.camera_fov * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad / 2.0f);

    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (app.camera_far + app.camera_near) / (app.camera_near - app.camera_far), -1,
        0, 0, (2 * app.camera_far * app.camera_near) / (app.camera_near - app.camera_far), 0};

    // Matriz de vista (cámara retrocedida)
    float view[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -app.camera_distance, 1};

    // Matriz modelo (con rotación)
    float model[16];
    calculate_rotation_model_matrix_y(rotation_angle, model);

    // Calcular MVP: Proyección * Vista * Modelo
    // Nota: OpenGL usa matrices column-major.
    // Al multiplicar matrices Row-Major (A * B) donde A y B son datos Column-Major,
    // estamos calculando effectively (A^T * B^T) = (B * A)^T.
    // Para obtener P * V en OpenGL (Col-Major), necesitamos calcular (P * V)^T en Row-Major.
    // (P * V)^T = V^T * P^T.
    // V^T es view[] interpretado Row-Major. P^T es proj[] interpretado Row-Major.
    // Por tanto: temp = Loop(view, proj) nos da (P * V) en formato OpenGL.

    // 1. Calcular PV = Proj * View
    float pv[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            pv[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
            {
                // Intercambiado proj y view para obtener el orden correcto
                pv[i * 4 + j] += view[i * 4 + k] * proj[k * 4 + j];
            }
        }
    }

    // 2. Calcular MVP = PV * Modelo
    // Loop(model, pv) nos da (PV * Model) en formato OpenGL
    float mvp[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mvp[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
            {
                mvp[i * 4 + j] += model[i * 4 + k] * pv[k * 4 + j];
            }
        }
    }

    // Pasar al shader
    if (gs->mvp_location != -1)
    {
        glUniformMatrix4fv(gs->mvp_location, 1, GL_FALSE, mvp);
    }

    glUseProgram(0);
}

#endif