#ifndef ENGINE_H
#define ENGINE_H

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Motor OpenGL 3.3"

#define CLEAR_COLOR_R (112.0f / 255.0f)
#define CLEAR_COLOR_G (113.0f / 255.0f)
#define CLEAR_COLOR_B (118.0f / 255.0f)
#define CLEAR_COLOR_A 1.0f

typedef struct
{
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLint mvp_location;
} GraphicsState;

// ============================================================================
// FUNCIONES DEL MOTOR
// ============================================================================

/**
 * Inicializa SDL y crea una ventana con contexto OpenGL
 */
static inline bool init_sdl(SDL_Window **window)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
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

    if (*window == NULL)
    {
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
static inline bool init_opengl(SDL_Window *window)
{
    // Crear contexto OpenGL
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL)
    {
        debug_log("ERROR SDL_GL_CreateContext: %s", SDL_GetError());
        return false;
    }
    debug_log("Contexto OpenGL creado");

    // Cargar funciones de OpenGL
    if (!load_opengl_functions())
    {
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
 * Limpia recursos
 */
static inline void cleanup(SDL_Window *window, GraphicsState *gs)
{
    debug_log("Limpiando recursos...");

    if (gs->program != 0)
    {
        glDeleteProgram(gs->program);
        debug_log("Programa eliminado");
    }

    if (gs->vao != 0)
    {
        glDeleteVertexArrays(1, &gs->vao);
        debug_log("VAO eliminado");
    }

    if (gs->vbo != 0)
    {
        glDeleteBuffers(1, &gs->vbo);
        debug_log("VBO eliminado");
    }

    if (window != NULL)
    {
        SDL_DestroyWindow(window);
        debug_log("Ventana destruida");
    }

    SDL_Quit();
    debug_log("SDL finalizado");
}

#endif