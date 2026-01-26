/*
 * model.h
 * Motor OpenGL 3.3 - Carga de modelos OBJ con materiales
 */

#ifndef MODEL_H
#define MODEL_H

#include <stddef.h> // Necesario para offsetof

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
 * Centra el modelo en el origen y lo escala para que encaje en el rango [-1, 1]
 */
static void normalizar_modelo(Vertice *vertices, unsigned int num_vertices)
{
    if (num_vertices == 0)
        return;

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

    float centro_x = (min_x + max_x) / 2.0f;
    float centro_y = (min_y + max_y) / 2.0f;
    float centro_z = (min_z + max_z) / 2.0f;

    float max_dim = max_x - min_x;
    if ((max_y - min_y) > max_dim)
        max_dim = max_y - min_y;
    if ((max_z - min_z) > max_dim)
        max_dim = max_z - min_z;

    float factor_escala = (max_dim > 0) ? (2.0f / max_dim) : 1.0f;

    for (unsigned int i = 0; i < num_vertices; i++)
    {
        vertices[i].x = (vertices[i].x - centro_x) * factor_escala;
        vertices[i].y = (vertices[i].y - centro_y) * factor_escala;
        vertices[i].z = (vertices[i].z - centro_z) * factor_escala;
    }
}

/**
 * Cargar modelo desde un archivo OBJ
 * Modificado para manejar correctamente normales, UVs y prevenir corrupción de memoria.
 */
static inline bool cargar_modelo(Modelo *out_modelo, Arena *mi_arena, const char *ruta)
{
    debug_log("Cargando modelo: %s", ruta);
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
        fastObjMaterial mat = mesh->materials[mesh->face_materials[f]];

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

                datos_gpu[curr_v].r = mat.Kd[0];
                datos_gpu[curr_v].g = mat.Kd[1];
                datos_gpu[curr_v].b = mat.Kd[2];
                datos_gpu[curr_v].ksr = mat.Ks[0];
                datos_gpu[curr_v].ksg = mat.Ks[1];
                datos_gpu[curr_v].ksb = mat.Ks[2];
                datos_gpu[curr_v].ns = mat.Ns;
                curr_v++;
            }
        }
        vert_offset += vertices_en_esta_cara;
    }

    normalizar_modelo(datos_gpu, total_render_vertices);

    glGenVertexArrays(1, &out_modelo->vao);
    glGenBuffers(1, &out_modelo->vbo);
    glBindVertexArray(out_modelo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_modelo->vbo);
    glBufferData(GL_ARRAY_BUFFER, bytes_necesarios, datos_gpu, GL_STATIC_DRAW);

    // Atributos: Pos(0), Norm(1), UV(2), Color(3), Spec(4), Shininess(5)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, nx));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, r));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, ksr));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void *)offsetof(Vertice, ns));
    glEnableVertexAttribArray(5);

    out_modelo->num_vertices = total_render_vertices;
    glBindVertexArray(0);
    fast_obj_destroy(mesh);
    return true;
}

/**
 * Setup Matrices: Ahora solo calcula Proyección * Vista.
 * El modelo se asume en la identidad (0,0,0) sin rotación.
 */
static inline void setup_matrices(GraphicsState *gs, int width, int height, AppState app)
{
    glUseProgram(gs->program);

    float aspect = (float)width / (float)height;
    float fov_rad = app.camera_fov * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad / 2.0f);

    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (app.camera_far + app.camera_near) / (app.camera_near - app.camera_far), -1,
        0, 0, (2 * app.camera_far * app.camera_near) / (app.camera_near - app.camera_far), 0};

    float view[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -app.camera_distance, 1};

    // MVP = Proj * View (ya que Model es Identidad)
    float mvp[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mvp[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
            {
                // Orden correcto para OpenGL column-major: View * Proj
                mvp[i * 4 + j] += view[i * 4 + k] * proj[k * 4 + j];
            }
        }
    }

    if (gs->mvp_location != -1)
    {
        glUniformMatrix4fv(gs->mvp_location, 1, GL_FALSE, mvp);
    }
}

#endif