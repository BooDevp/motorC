/**
 * Motor Gráfico Simple con OpenGL 3.3 y SDL3 - VERSIÓN WINDOWS
 * Muestra un cubo estático en el centro de la pantalla
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>

// Incluir OpenGL headers para Windows
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

// ============================================================================
// CONSTANTES CONFIGURABLES
// ============================================================================

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
#define WINDOW_TITLE    "Motor OpenGL 3.3 - Cubo Estático"

#define CAMERA_FOV      45.0f
#define CAMERA_NEAR     0.1f
#define CAMERA_FAR      100.0f
#define CAMERA_DISTANCE 5.0f

#define CLEAR_COLOR_R   0.1f
#define CLEAR_COLOR_G   0.15f
#define CLEAR_COLOR_B   0.2f
#define CLEAR_COLOR_A   1.0f

// ============================================================================
// PROTOTIPOS DE FUNCIONES OPENGL
// ============================================================================

// Tipos básicos (si no están definidos)
#ifndef GLuint
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#endif

// Constantes de OpenGL 3.3+ (para Windows)
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

// Funciones de shaders (las cargaremos dinámicamente)
typedef GLuint (__stdcall *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (__stdcall *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
typedef void (__stdcall *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (__stdcall *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (__stdcall *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (__stdcall *PFNGLDELETESHADERPROC)(GLuint shader);

// Funciones de programas
typedef GLuint (__stdcall *PFNGLCREATEPROGRAMPROC)(void);
typedef void (__stdcall *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (__stdcall *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (__stdcall *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (__stdcall *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (__stdcall *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (__stdcall *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint (__stdcall *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void (__stdcall *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

// Funciones de buffers y VAOs
typedef void (__stdcall *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (__stdcall *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (__stdcall *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (__stdcall *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (__stdcall *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (__stdcall *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (__stdcall *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (__stdcall *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (__stdcall *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);

// ============================================================================
// VARIABLES GLOBALES OPENGL
// ============================================================================

PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;

PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;

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
    bool show_debug;
    bool running;
} AppState;

// ============================================================================
// SHADERS EMBEBIDOS
// ============================================================================

static const char* VERTEX_SHADER_SOURCE = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aColor;\n"
    "uniform mat4 uMVP;\n"
    "out vec3 fragColor;\n"
    "void main() {\n"
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n"
    "    fragColor = aColor;\n"
    "}\n";

static const char* FRAGMENT_SHADER_SOURCE = 
    "#version 330 core\n"
    "in vec3 fragColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vec4(fragColor, 1.0);\n"
    "}\n";

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void debug_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

// ============================================================================
// FUNCIONES DE CARGA OPENGL
// ============================================================================

/**
 * Carga todas las funciones de OpenGL usando SDL_GL_GetProcAddress
 */
bool load_opengl_functions(void) {
    debug_log("Cargando funciones de OpenGL...");
    
    // Funciones de shaders
    glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    
    // Funciones de programas
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    
    // Funciones de buffers y VAOs
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    
    // Verificar funciones críticas
    if (!glCreateShader || !glCreateProgram || !glGenVertexArrays || !glGenBuffers) {
        debug_log("ERROR: No se pudieron cargar funciones críticas de OpenGL");
        return false;
    }
    
    debug_log("Funciones de OpenGL cargadas correctamente");
    return true;
}

// ============================================================================
// FUNCIONES DEL MOTOR
// ============================================================================

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
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    
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
 * Crea la geometría del cubo
 */
bool create_cube_geometry(GraphicsState* gs) {
    // Vértices del cubo: posición (x,y,z) + color (r,g,b)
    float vertices[] = {
        // Cara frontal (roja)
        -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
        
        // Cara trasera (verde)
        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        
        // Cara superior (azul)
        -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        
        // Cara inferior (amarilla)
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
        
        // Cara derecha (cian)
         1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        
        // Cara izquierda (magenta)
        -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f
    };
    
    // Crear VAO
    glGenVertexArrays(1, &gs->vao);
    glBindVertexArray(gs->vao);
    
    // Crear VBO
    glGenBuffers(1, &gs->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gs->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Atributo 0: Posición (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Atributo 1: Color (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Desvincular
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    debug_log("Geometría creada: VAO=%u, VBO=%u, %d vértices", 
              gs->vao, gs->vbo, sizeof(vertices) / (6 * sizeof(float)));
    return true;
}

/**
 * Configura las matrices (MVP)
 */
void setup_matrices(GraphicsState* gs, int width, int height) {
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
    
    // Matriz modelo (identidad - cubo en origen)
    float model[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
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
    debug_log("Matrices configuradas: %dx%d, FOV=%.1f, Distancia=%.1f", width, height, CAMERA_FOV, CAMERA_DISTANCE);
}

/**
 * Renderiza un frame
 */
void render_frame(GraphicsState* gs, AppState* app) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (app->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glUseProgram(gs->program);
    glBindVertexArray(gs->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vértices
    
    glBindVertexArray(0);
    glUseProgram(0);
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

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("MOTOR OPENGL 3.3 - CUBO ESTÁTICO (Windows)\n");
    printf("========================================\n\n");
    
    AppState app = {
        .wireframe = false,
        .show_debug = true,
        .running = true
    };
    
    GraphicsState gs = {0};
    SDL_Window* window = NULL;
    
    // 1. INICIALIZAR SDL
    debug_log("Paso 1: Inicializando SDL3...");
    if (!init_sdl(&window)) {
        return 1;
    }
    
    // 2. INICIALIZAR OPENGL
    debug_log("Paso 2: Inicializando OpenGL...");
    if (!init_opengl(window)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // 3. CREAR SHADERS
    debug_log("Paso 3: Creando shaders...");
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
    
    // 4. CREAR GEOMETRÍA
    debug_log("Paso 4: Creando geometría...");
    if (!create_cube_geometry(&gs)) {
        glDeleteProgram(gs.program);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // 5. CONFIGURAR MATRICES
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    setup_matrices(&gs, width, height);
    
    debug_log("\n========================================");
    debug_log("MOTOR LISTO");
    debug_log("Controles:");
    debug_log("  ESC - Salir");
    debug_log("  F1  - Alternar wireframe");
    debug_log("========================================\n");
    
    // 6. BUCLE PRINCIPAL
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
                setup_matrices(&gs, width, height);
                if (app.show_debug) {
                    debug_log("Ventana redimensionada: %dx%d", width, height);
                }
            }
        }
        
        render_frame(&gs, &app);
        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }
    
    // 7. LIMPIEZA
    debug_log("\nFinalizando...");
    cleanup(window, &gs);
    
    printf("\nAplicación terminada correctamente.\n");
    return 0;
}