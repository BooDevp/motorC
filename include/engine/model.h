/*
 * model.h
 * Motor OpenGL 3.3 - Carga de modelos OBJ con materiales y shaders
 */

#ifndef MODEL_H
#define MODEL_H

#define FAST_OBJ_IMPLEMENTATION
#include "external/fast_obj.h"

#include <stddef.h>
#include <math.h>
#include <string.h>

#include "shader.h"

#define TO_RAD (SDL_PI_F / 180.0f)

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

// Sistema de parámetros para shaders
typedef enum
{
    SHADER_PARAM_FLOAT,
    SHADER_PARAM_VEC2,
    SHADER_PARAM_VEC3,
    SHADER_PARAM_VEC4,
    SHADER_PARAM_INT
} ShaderParamType;

typedef struct
{
    char name[32];
    ShaderParamType type;
    union
    {
        float f;
        float vec2[2];
        float vec3[3];
        float vec4[4];
        int i;
    } value;
} ShaderParam;

// Estructura para manejar el modelo en el motor
typedef struct
{
    GLuint vao;
    GLuint vbo;
    int num_vertices;

    float posicion[3];
    float escala[3];
    float rotacion[3];

    GLuint shader; // 0 = default, >0 = custom shader program
    GLint mvp_location;

    GLuint textura_id;

    // Caché de uniform locations para optimización
    GLint time_location;
    GLint texture_location;

    // Parámetros dinámicos para el shader
    ShaderParam *params;
    int num_params;
    int max_params;

    // Controller asociado (opcional, puede ser NULL)
    void *controller;
    void (*controller_update)(void *ctrl, float delta_time, void *modelo);
} Modelo;

/**
 * Funciones para establecer parámetros de shader en modelos
 */

// Función auxiliar para encontrar o crear un parámetro
static ShaderParam *modelo_get_or_create_param(Modelo *m, const char *name)
{
    // Buscar si ya existe
    for (int i = 0; i < m->num_params; i++)
    {
        if (strcmp(m->params[i].name, name) == 0)
        {
            return &m->params[i];
        }
    }

    // Si no existe, crear uno nuevo
    if (m->num_params >= m->max_params)
    {
        // Necesitamos expandir el array
        int new_max = (m->max_params == 0) ? 4 : m->max_params * 2;
        ShaderParam *new_params = (ShaderParam *)SDL_realloc(m->params, new_max * sizeof(ShaderParam));

        if (!new_params)
        {
            debug_log("ERROR: No se pudo asignar memoria para parámetros de shader");
            return NULL;
        }

        m->params = new_params;
        m->max_params = new_max;
    }

    // Crear nuevo parámetro
    ShaderParam *param = &m->params[m->num_params];
    strncpy(param->name, name, 31);
    param->name[31] = '\0';
    m->num_params++;

    return param;
}

// Establecer parámetro float
static inline void modelo_set_float(Modelo *m, const char *name, float value)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_FLOAT;
        param->value.f = value;
    }
}

// Establecer parámetro vec2
static inline void modelo_set_vec2(Modelo *m, const char *name, float x, float y)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC2;
        param->value.vec2[0] = x;
        param->value.vec2[1] = y;
    }
}

// Establecer parámetro vec3
static inline void modelo_set_vec3(Modelo *m, const char *name, float x, float y, float z)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC3;
        param->value.vec3[0] = x;
        param->value.vec3[1] = y;
        param->value.vec3[2] = z;
    }
}

// Establecer parámetro vec4
static inline void modelo_set_vec4(Modelo *m, const char *name, float x, float y, float z, float w)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_VEC4;
        param->value.vec4[0] = x;
        param->value.vec4[1] = y;
        param->value.vec4[2] = z;
        param->value.vec4[3] = w;
    }
}

// Establecer parámetro int
static inline void modelo_set_int(Modelo *m, const char *name, int value)
{
    ShaderParam *param = modelo_get_or_create_param(m, name);
    if (param)
    {
        param->type = SHADER_PARAM_INT;
        param->value.i = value;
    }
}

// Aplicar todos los parámetros al shader actual
static inline void modelo_apply_shader_params(Modelo *m)
{
    if (!m->params || m->num_params == 0)
        return;

    GLuint current_program = (m->shader != 0) ? m->shader : 0;
    if (current_program == 0)
        return;

    for (int i = 0; i < m->num_params; i++)
    {
        ShaderParam *param = &m->params[i];

        switch (param->type)
        {
        case SHADER_PARAM_FLOAT:
            shader_set_float(current_program, param->name, param->value.f);
            break;
        case SHADER_PARAM_VEC2:
            shader_set_vec2(current_program, param->name,
                            param->value.vec2[0], param->value.vec2[1]);
            break;
        case SHADER_PARAM_VEC3:
            shader_set_vec3(current_program, param->name,
                            param->value.vec3[0], param->value.vec3[1], param->value.vec3[2]);
            break;
        case SHADER_PARAM_VEC4:
            shader_set_vec4(current_program, param->name,
                            param->value.vec4[0], param->value.vec4[1],
                            param->value.vec4[2], param->value.vec4[3]);
            break;
        case SHADER_PARAM_INT:
            shader_set_int(current_program, param->name, param->value.i);
            break;
        }
    }
}

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

                // --- PARA LAS UVs ---
                if (mesh->texcoord_count > 1)
                {
                    datos_gpu[curr_v].u = mesh->texcoords[idx.t * 2 + 0];
                    datos_gpu[curr_v].v = mesh->texcoords[idx.t * 2 + 1];
                }

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

    // Inicializacion del modelo
    out_modelo->num_vertices = total_render_vertices;

    out_modelo->posicion[0] = 0.0f;
    out_modelo->posicion[1] = 0.0f;
    out_modelo->posicion[2] = 0.0f;

    out_modelo->escala[0] = 1.0f;
    out_modelo->escala[1] = 1.0f;
    out_modelo->escala[2] = 1.0f;

    out_modelo->rotacion[0] = 0.0f;
    out_modelo->rotacion[1] = 0.0f;
    out_modelo->rotacion[2] = 0.0f;

    out_modelo->shader = 0;        // Por defecto usa el shader global
    out_modelo->mvp_location = -1; // No cached yet

    // Inicializar caché de uniform locations
    out_modelo->time_location = -1;
    out_modelo->texture_location = -1;

    // Inicializar sistema de parámetros de shader
    out_modelo->params = NULL;
    out_modelo->num_params = 0;
    out_modelo->max_params = 0;

    // Inicializar controller (opcional)
    out_modelo->controller = NULL;
    out_modelo->controller_update = NULL;

    glBindVertexArray(0);
    fast_obj_destroy(mesh);
    return true;
}

/**
 * Setup Matrices: Calcula MVP = Proj * View * Model
 * El modelo aplica transformaciones en orden: Escala -> Rotación (YXZ) -> Traslación
 */
static inline void setup_matrices(GraphicsState *gs, int width, int height, Modelo *m, Camara *cam)
{
    // Usar shader propio si existe, si no el global
    GLuint current_program = (m->shader != 0) ? m->shader : gs->program;
    glUseProgram(current_program);

    // Obtener Proyección desde la Cámara
    float proj[16];
    calcular_matriz_proyeccion(cam, width, height, proj);

    // Obtener Vista desde la Cámara
    float view[16];
    calcular_matriz_vista(cam, view);

    // Matriz de Modelo (se mantiene igual, depende del objeto)
    float radX = m->rotacion[0] * TO_RAD;
    float radY = m->rotacion[1] * TO_RAD;
    float radZ = m->rotacion[2] * TO_RAD;
    float mcx = cosf(radX), msx = sinf(radX);
    float mcy = cosf(radY), msy = sinf(radY);
    float mcz = cosf(radZ), msz = sinf(radZ);

    float model[16] = {
        m->escala[0] * (mcy * mcz + msy * msx * msz), m->escala[1] * (mcx * msz), m->escala[2] * (-msy * mcz + mcy * msx * msz), 0,
        m->escala[0] * (-mcy * msz + msy * msx * mcz), m->escala[1] * (mcx * mcz), m->escala[2] * (msy * msz + mcy * msx * mcz), 0,
        m->escala[0] * (msy * mcx), m->escala[1] * (-msx), m->escala[2] * (mcy * mcx), 0,
        m->posicion[0], m->posicion[1], m->posicion[2], 1};

    // Multiplicación MVP (Proceso estándar)
    float pv[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            pv[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
                pv[i * 4 + j] += view[i * 4 + k] * proj[k * 4 + j];
        }
    }

    float mvp[16];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mvp[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
                mvp[i * 4 + j] += model[i * 4 + k] * pv[k * 4 + j];
        }
    }

    // Si usamos un shader custom, necesitamos buscar su uniform location
    // Si usamos el global, usamos la optimización gs->mvp_location
    GLint loc = -1;
    if (m->shader != 0)
    {
        loc = m->mvp_location;
    }
    else
    {
        loc = gs->mvp_location;
    }

    if (loc != -1)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, mvp);
    }

    // Activar textura de máscara si el modelo la tiene
    if (m->textura_id != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m->textura_id);

        // OPTIMIZACIÓN: Usar caché en lugar de glGetUniformLocation
        if (m->texture_location != -1)
        {
            glUniform1i(m->texture_location, 0);
        }
    }

    // OPTIMIZACIÓN: Usar caché en lugar de glGetUniformLocation
    if (m->time_location != -1)
    {
        glUniform1f(m->time_location, (float)SDL_GetTicks() / 1000.0f);
    }

    // Aplicar parámetros personalizados del modelo
    modelo_apply_shader_params(m);
}

#endif