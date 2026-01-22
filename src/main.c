#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gl_funcs.h"

// Función para leer archivos (Flujo Disco -> RAM)
char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Error: No se pudo abrir %s\n", filename);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

// Función para chequear errores de compilación de shaders
void check_shader_error(GLuint shader, const char* type) {
    GLint success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n", type, infoLog);
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Motor C - Triangulo Animado", 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!init_gl_funcs()) {
        printf("Error al inicializar funciones de OpenGL\n");
        return 1;
    }

    // --- CARGA DE SHADERS ---
    char* vert_src = read_file("src/shaders/simple.vert");
    char* frag_src = read_file("src/shaders/simple.frag");
    if (!vert_src || !frag_src) return 1;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, (const char**)&vert_src, NULL);
    glCompileShader(vs);
    check_shader_error(vs, "VERTEX");

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, (const char**)&frag_src, NULL);
    glCompileShader(fs);
    check_shader_error(fs, "FRAGMENT");

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    free(vert_src);
    free(frag_src);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // --- DATOS DEL TRIÁNGULO (Posicion XYZ + Color RGB) ---
    float vertices[] = {
        // Posiciones          // Colores
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // Rojo
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // Verde
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // Azul
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo 0: Posición (3 floats, salto de 6)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Color (3 floats, empieza en el byte 12)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- BUCLE PRINCIPAL ---
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = 0;
        }

        // Fondo oscuro
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        // ENVIAR TIEMPO A LA GPU (La "Pipe" de Uniforms)
        float timeValue = SDL_GetTicks() / 1000.0f;
        int timeLoc = glGetUniformLocation(program, "u_time");
        glUniform1f(timeLoc, timeValue);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}