/**
 * Motor Gráfico Simple con OpenGL 3.3 y SDL3 - VERSIÓN WINDOWS
 * Muestra un cubo estático en el centro de la pantalla
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define GL_IMPLEMENTATION
#include "gl_headers.h"

#include "gestion_memoria.h"
#include "shader.h"

// ============================================================================
// CONSTANTES CONFIGURABLES
// ============================================================================

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Motor OpenGL 3.3"

#define CAMERA_FOV 45.0f
#define CAMERA_NEAR 0.1f
#define CAMERA_FAR 100.0f
#define CAMERA_DISTANCE 4.0f

#define CLEAR_COLOR_R (0.0f / 255.0f)
#define CLEAR_COLOR_G (51.0f / 255.0f)
#define CLEAR_COLOR_B (204.0f / 255.0f)
#define CLEAR_COLOR_A 1.0f

#define ARENA_SIZE_MB 10

// ============================================================================
// ESTRUCTURAS
// ============================================================================

typedef struct
{
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLint mvp_location;
} GraphicsState;

typedef struct
{
    bool wireframe;
    bool running;    
    float camera_fov;
    float camera_near;
    float camera_far;
    float camera_distance;
} AppState;

#include "model.h"

// ============================================================================
// FUNCIONES DEL MOTOR
// ============================================================================

/**
 * Inicializa SDL y crea una ventana con contexto OpenGL
 */
bool init_sdl(SDL_Window **window)
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
bool init_opengl(SDL_Window *window)
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
void cleanup(SDL_Window *window, GraphicsState *gs)
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

/**
 * Renderiza un frame
 */
void render_frame(GraphicsState *gs, AppState *app, Modelo *modelo)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (app->wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glUseProgram(gs->program);
    glBindVertexArray(modelo->vao);                      // <--- Usamos el VAO del modelo cargado
    glDrawArrays(GL_TRIANGLES, 0, modelo->num_vertices); // <--- Usamos su número de vértices

    glBindVertexArray(0);
    glUseProgram(0);
}

// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main(int argc, char *argv[])
{
    (void)argc; // Silenciar el warning
    (void)argv; // Silenciar el warning
    printf("========================================\n");
    printf("MOTOR OPENGL 3.3 (Windows)\n");
    printf("========================================\n\n");

    AppState app = {
        .wireframe = false,
        .running = true,        
        .camera_fov = CAMERA_FOV,
        .camera_near = CAMERA_NEAR,
        .camera_far = CAMERA_FAR,
        .camera_distance = CAMERA_DISTANCE,
    };

    GraphicsState gs = {0};
    SDL_Window *window = NULL;

    // GESTIÓN DE MEMORIA
    Arena arena_escena;
    arena_inicializar(&arena_escena, ARENA_SIZE_MB * 1024 * 1024);

    // INICIALIZAR SDL
    debug_log("Inicializando SDL3...");
    if (!init_sdl(&window))
    {
        return 1;
    }

    // INICIALIZAR OPENGL
    debug_log("Inicializando OpenGL...");
    if (!init_opengl(window))
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // CREAR SHADERS
    debug_log("Creando shaders...");
    gs.program = create_shader_program();
    if (gs.program == 0)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Obtener ubicación del uniform
    gs.mvp_location = glGetUniformLocation(gs.program, "uMVP");
    if (gs.mvp_location == -1)
    {
        debug_log("ADVERTENCIA: Uniform uMVP no encontrado (puede estar optimizado)");
    }
    else
    {
        debug_log("Uniform uMVP ubicado en: %d", gs.mvp_location);
    }

    Modelo mi_cubo_obj;
    if (cargar_modelo(&mi_cubo_obj, &arena_escena, "assets/models/Icecream.obj"))
    {
        printf("¡Modelo cargado con éxito!\n");
    }

    // CONFIGURAR MATRICES
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    setup_matrices(&gs, width, height, app);

    debug_log("\n========================================");
    debug_log("MOTOR LISTO");
    debug_log("Controles:");
    debug_log("  ESC - Salir");
    debug_log("  F1  - Alternar wireframe");
    debug_log("========================================\n");

    // BUCLE PRINCIPAL
    debug_log("Iniciando bucle de renderizado...");
    while (app.running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                app.running = false;
                debug_log("Evento QUIT recibido");
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE)
                {
                    app.running = false;
                    debug_log("Tecla ESC presionada");
                }

                if (event.key.key == SDLK_F1)
                {
                    app.wireframe = !app.wireframe;
                    debug_log("Wireframe: %s", app.wireframe ? "ON" : "OFF");
                }
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSizeInPixels(window, &width, &height);
                glViewport(0, 0, width, height);
                setup_matrices(&gs, width, height, app);
                debug_log("Ventana redimensionada: %dx%d", width, height);
            }
        }        

        // Actualizar matrices cada frame
        setup_matrices(&gs, width, height, app);

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