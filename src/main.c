/**
 * Motor Gráfico Simple con OpenGL 3.3 y SDL3 - VERSIÓN WINDOWS
 * Muestra un cubo estático en el centro de la pantalla
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "gestion_memoria.h"

#define FAST_OBJ_IMPLEMENTATION 
#include "fast_obj.h"

#define GL_IMPLEMENTATION
#include "gl_headers.h"

// ============================================================================
// CONSTANTES CONFIGURABLES
// ============================================================================

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define WINDOW_TITLE    "Motor OpenGL 3.3"

#define CAMERA_FOV      45.0f
#define CAMERA_NEAR     0.1f
#define CAMERA_FAR      100.0f
#define CAMERA_DISTANCE 4.0f

#define CLEAR_COLOR_R   (0.0f   / 255.0f)
#define CLEAR_COLOR_G   (51.0f  / 255.0f)
#define CLEAR_COLOR_B   (204.0f / 255.0f)
#define CLEAR_COLOR_A   1.0f

#define ROTATION_SPEED_Y 0.25f

#define ARENA_SIZE_MB 10

// ============================================================================
// ESTRUCTURAS
// ============================================================================

typedef struct {
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLint mvp_location;
} GraphicsState;

typedef struct {
    bool wireframe;    
    bool running;
    float rotation_y;
} AppState;

// ============================================================================
// FUNCIONES DEL MOTOR
// ============================================================================

/**
 * Carga el código fuente de un shader desde un archivo
 */
char* load_shader_source(const char* filename) {
    size_t dataSize = 0;
    void* data = SDL_LoadFile(filename, &dataSize);
    
    if (data == NULL) {
        debug_log("ERROR cargando archivo shader %s: %s", filename, SDL_GetError());
        return NULL;
    }
    
    // SDL_LoadFile data is guaranteed to be null-terminated in SDL3
    return (char*)data;
}

/**
 * Inicializa SDL y crea una ventana con contexto OpenGL
 */
bool init_sdl(SDL_Window** window) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        debug_log("ERROR SDL_Init: %s", SDL_GetError());
        return false;
    }
    debug_log("SDL3 inicializado");
    
    // Configurar atributos de OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Crear ventana
    *window = SDL_CreateWindow(WINDOW_TITLE, 
                               WINDOW_WIDTH, 
                               WINDOW_HEIGHT, 
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    if (*window == NULL) {
        debug_log("ERROR SDL_CreateWindow: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }
    debug_log("Ventana creada: %dx%d", WINDOW_WIDTH, WINDOW_HEIGHT);
    
    return true;
}

/**
 * Inicializa OpenGL con configuraciones básicas
 */
bool init_opengl(SDL_Window* window) {
    // Crear contexto OpenGL
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        debug_log("ERROR SDL_GL_CreateContext: %s", SDL_GetError());
        return false;
    }
    debug_log("Contexto OpenGL creado");
    
    // Cargar funciones de OpenGL
    if (!load_opengl_functions()) {
        SDL_GL_DestroyContext(gl_context);
        return false;
    }
    
    // Configurar viewport
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // Configurar estado de OpenGL
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
    
    debug_log("OpenGL configurado: Depth Test ON, Culling ON");
    return true;
}

/**
 * Compila un shader individual
 */
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Verificar errores
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        debug_log("ERROR compilando shader: %s", info_log);
        return 0;
    }
    
    return shader;
}

/**
 * Crea un programa de shaders
 */
GLuint create_shader_program(void) {
    char* vertex_source = load_shader_source("src/shaders/simple.vert");
    char* fragment_source = load_shader_source("src/shaders/simple.frag");
    
    if (!vertex_source || !fragment_source) {
        if (vertex_source) SDL_free(vertex_source);
        if (fragment_source) SDL_free(fragment_source);
        return 0;
    }

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    
    SDL_free(vertex_source);
    SDL_free(fragment_source);

    if (vertex_shader == 0 || fragment_shader == 0) {
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // Verificar linking
    GLint success;
    GLchar info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        debug_log("ERROR linkando programa: %s", info_log);
        glDeleteProgram(program);
        return 0;
    }
    
    // Limpiar shaders individuales
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    debug_log("Programa de shaders creado (ID: %u)", program);
    return program;
}

/**
 * Crea una matriz de rotación sobre el eje Y
 * Didáctico: Explica cómo se construye la matriz
 */
void calculate_rotation_matrix_y(float angle_deg, float* matrix) {
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
void setup_matrices(GraphicsState* gs, int width, int height, float rotation_angle) {
    glUseProgram(gs->program);
    
    // Matriz de proyección (perspectiva)
    float aspect = (float)width / (float)height;
    float fov_rad = CAMERA_FOV * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad / 2.0f);
    
    float proj[16] = {
        f/aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (CAMERA_FAR + CAMERA_NEAR) / (CAMERA_NEAR - CAMERA_FAR), -1,
        0, 0, (2 * CAMERA_FAR * CAMERA_NEAR) / (CAMERA_NEAR - CAMERA_FAR), 0
    };
    
    // Matriz de vista (cámara retrocedida)
    float view[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -CAMERA_DISTANCE, 1
    };
    
    // Matriz modelo (con rotación)
    float model[16];
    calculate_rotation_matrix_y(rotation_angle, model);
    
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
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pv[i*4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                // Intercambiado proj y view para obtener el orden correcto
                pv[i*4 + j] += view[i*4 + k] * proj[k*4 + j];
            }
        }
    }

    // 2. Calcular MVP = PV * Modelo
    // Loop(model, pv) nos da (PV * Model) en formato OpenGL
    float mvp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mvp[i*4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                mvp[i*4 + j] += model[i*4 + k] * pv[k*4 + j];
            }
        }
    }
    
    // Pasar al shader
    if (gs->mvp_location != -1) {
        glUniformMatrix4fv(gs->mvp_location, 1, GL_FALSE, mvp);
    }
    
    glUseProgram(0);    
    // debug_log("Matrices configuradas..."); // Comentado para no saturar el log cada frame
}

/**
 * Limpia recursos
 */
void cleanup(SDL_Window* window, GraphicsState* gs) {
    debug_log("Limpiando recursos...");
    
    if (gs->program != 0) {
        glDeleteProgram(gs->program);
        debug_log("Programa eliminado");
    }
    
    if (gs->vao != 0) {
        glDeleteVertexArrays(1, &gs->vao);
        debug_log("VAO eliminado");
    }
    
    if (gs->vbo != 0) {
        glDeleteBuffers(1, &gs->vbo);
        debug_log("VBO eliminado");
    }
    
    if (window != NULL) {
        SDL_DestroyWindow(window);
        debug_log("Ventana destruida");
    }
    
    SDL_Quit();
    debug_log("SDL finalizado");
}

// Estructura de vértice "Interleaved" (todo junto para la GPU)
typedef struct {
    float x, y, z;    // Posición
    float nx, ny, nz; // Normales
    float u, v;       // UVs
    float r, g, b;    // Color Difuso (Kd)
    float ksr, ksg, ksb; // Color Especular (Ks)
    float ns;         // Exponente de brillo (Ns)
} Vertice;

// Estructura para manejar el modelo en el motor
typedef struct {
    GLuint vao;
    GLuint vbo;
    int num_vertices;
} Modelo;

/**
 * Renderiza un frame
 */
void render_frame(GraphicsState* gs, AppState* app, Modelo* modelo) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (app->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glUseProgram(gs->program);
    glBindVertexArray(modelo->vao); // <--- Usamos el VAO del modelo cargado
    glDrawArrays(GL_TRIANGLES, 0, modelo->num_vertices); // <--- Usamos su número de vértices
    
    glBindVertexArray(0);
    glUseProgram(0);
}

/**
 * Centra el modelo en el origen (0,0,0) y lo escala a un tamaño estándar.
 */
void centrar_modelo(Vertice* vertices, unsigned int num_vertices) {
    if (num_vertices == 0) return;

    // 1. Encontrar los límites (Bounding Box)
    float min_x = 1e10, max_x = -1e10;
    float min_y = 1e10, max_y = -1e10;
    float min_z = 1e10, max_z = -1e10;

    for (unsigned int i = 0; i < num_vertices; i++) {
        if (vertices[i].x < min_x) min_x = vertices[i].x;
        if (vertices[i].x > max_x) max_x = vertices[i].x;
        if (vertices[i].y < min_y) min_y = vertices[i].y;
        if (vertices[i].y > max_y) max_y = vertices[i].y;
        if (vertices[i].z < min_z) min_z = vertices[i].z;
        if (vertices[i].z > max_z) max_z = vertices[i].z;
    }

    // 2. Calcular el centro y la dimensión mayor
    float centro_x = (min_x + max_x) / 2.0f;
    float centro_y = (min_y + max_y) / 2.0f;
    float centro_z = (min_z + max_z) / 2.0f;

    float size_x = max_x - min_x;
    float size_y = max_y - min_y;
    float size_z = max_z - min_z;
    
    float max_dim = size_x;
    if (size_y > max_dim) max_dim = size_y;
    if (size_z > max_dim) max_dim = size_z;

    // Queremos que el objeto encaje en un cubo de tamaño 2 (-1 a 1)
    float factor_escala = (max_dim > 0) ? (2.0f / max_dim) : 1.0f;

    // 3. Aplicar transformación a cada vértice
    for (unsigned int i = 0; i < num_vertices; i++) {
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
bool cargar_modelo(Modelo* out_modelo, Arena* mi_arena, const char* ruta) {
    debug_log("Cargando modelo con Materiales Completos: %s", ruta);
    
    fastObjMesh* mesh = fast_obj_read(ruta);
    if (!mesh) return false;

    unsigned int total_render_vertices = 0;
    for (unsigned int f = 0; f < mesh->face_count; f++) {
        total_render_vertices += (mesh->face_vertices[f] - 2) * 3;
    }

    size_t bytes_necesarios = sizeof(Vertice) * total_render_vertices;
    Vertice* datos_gpu = (Vertice*)arena_push(mi_arena, bytes_necesarios);
    if (!datos_gpu) { fast_obj_destroy(mesh); return false; }

    unsigned int curr_v = 0;
    unsigned int vert_offset = 0;

    for (unsigned int f = 0; f < mesh->face_count; f++) {
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

        for (unsigned int v = 1; v < vertices_en_esta_cara - 1; v++) {
            unsigned int face_indices[3] = {0, v, v + 1};
            for (int i = 0; i < 3; i++) {
                fastObjIndex idx = mesh->indices[vert_offset + face_indices[i]];

                datos_gpu[curr_v].x = mesh->positions[idx.p * 3 + 0];
                datos_gpu[curr_v].y = mesh->positions[idx.p * 3 + 1];
                datos_gpu[curr_v].z = mesh->positions[idx.p * 3 + 2];
                
                if (mesh->normal_count > 1) {
                    datos_gpu[curr_v].nx = mesh->normals[idx.n * 3 + 0];
                    datos_gpu[curr_v].ny = mesh->normals[idx.n * 3 + 1];
                    datos_gpu[curr_v].nz = mesh->normals[idx.n * 3 + 2];
                }

                // Asignar datos de material al vértice
                datos_gpu[curr_v].r = dr; datos_gpu[curr_v].g = dg; datos_gpu[curr_v].b = db;
                datos_gpu[curr_v].ksr = sr; datos_gpu[curr_v].ksg = sg; datos_gpu[curr_v].ksb = sb;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Location 3: Color Difuso (Kd)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Location 4: Color Especular (Ks)
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);
    
    // Location 5: Shininess (Ns)
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);

    out_modelo->num_vertices = total_render_vertices;
    glBindVertexArray(0);
    fast_obj_destroy(mesh); 
    return true;
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char* argv[]) {
    (void)argc; // Silenciar el warning
    (void)argv; // Silenciar el warning
    printf("========================================\n");
    printf("MOTOR OPENGL 3.3 (Windows)\n");
    printf("========================================\n\n");
    
    AppState app = {
        .wireframe = false,
        .running = true,
        .rotation_y = 0.0f
    };
    
    GraphicsState gs = {0};
    SDL_Window* window = NULL;

    // GESTIÓN DE MEMORIA    
    Arena arena_escena;
    arena_inicializar(&arena_escena, ARENA_SIZE_MB * 1024 * 1024);
    arena_reporte(&arena_escena, "INICIALIZACION");    
    
    // INICIALIZAR SDL
    debug_log("Inicializando SDL3...");
    if (!init_sdl(&window)) {
        return 1;
    }
    
    // INICIALIZAR OPENGL
    debug_log("Inicializando OpenGL...");
    if (!init_opengl(window)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // CREAR SHADERS
    debug_log("Creando shaders...");
    gs.program = create_shader_program();
    if (gs.program == 0) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Obtener ubicación del uniform
    gs.mvp_location = glGetUniformLocation(gs.program, "uMVP");
    if (gs.mvp_location == -1) {
        debug_log("ADVERTENCIA: Uniform uMVP no encontrado (puede estar optimizado)");
    } else {
        debug_log("Uniform uMVP ubicado en: %d", gs.mvp_location);
    }

    Modelo mi_cubo_obj;
    if (cargar_modelo(&mi_cubo_obj, &arena_escena, "assets/models/GatoLowPoly.obj")) {
        printf("¡Modelo cargado con éxito!\n");
    }
    
    // CONFIGURAR MATRICES
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    setup_matrices(&gs, width, height, app.rotation_y);
    
    debug_log("\n========================================");
    debug_log("MOTOR LISTO");
    debug_log("Controles:");
    debug_log("  ESC - Salir");
    debug_log("  F1  - Alternar wireframe");
    debug_log("========================================\n");
    
    // BUCLE PRINCIPAL
    debug_log("Iniciando bucle de renderizado...");
    while (app.running) {
        SDL_Event event;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                app.running = false;
                debug_log("Evento QUIT recibido");
            }
            
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    app.running = false;
                    debug_log("Tecla ESC presionada");
                }
                
                if (event.key.key == SDLK_F1) {
                    app.wireframe = !app.wireframe;
                    debug_log("Wireframe: %s", app.wireframe ? "ON" : "OFF");
                }
            }
            
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSizeInPixels(window, &width, &height);
                glViewport(0, 0, width, height);
                setup_matrices(&gs, width, height, app.rotation_y);
                debug_log("Ventana redimensionada: %dx%d", width, height);
            }
        }
        
        // Actualizar rotación
        app.rotation_y += ROTATION_SPEED_Y;
        if (app.rotation_y >= 360.0f) app.rotation_y -= 360.0f;
        
        // Actualizar matrices cada frame
        setup_matrices(&gs, width, height, app.rotation_y);
        
        render_frame(&gs, &app, &mi_cubo_obj);
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }
    
    // LIMPIEZA
    debug_log("\nFinalizando...");
    cleanup(window, &gs);
    
    free(arena_escena.base);
    debug_log("Arena liberada!");

    printf("\nAplicación terminada correctamente.\n");
    return 0;
}