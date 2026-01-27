#ifndef POSTPROCESADO_H
#define POSTPROCESADO_H

#include "external/gl_headers.h"
#include "shader.h"
#include <stdio.h>

typedef struct
{
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
    GLuint vao;
    GLuint vbo;
    GLuint program;
    int width;
    int height;
    GLint timeLoc;
    GLint resLoc;
} PostProcessSystem;

// Recrea los buffers cuando cambia el tamaño de la ventana
void post_setup_buffers(PostProcessSystem *pp, int w, int h)
{
    pp->width = w;
    pp->height = h;

    // Limpieza de buffers antiguos si existen
    if (pp->fbo != 0) glDeleteFramebuffers(1, &pp->fbo);
    if (pp->texture != 0) glDeleteTextures(1, &pp->texture);
    if (pp->rbo != 0) glDeleteRenderbuffers(1, &pp->rbo);

    // Crear el Framebuffer Object (FBO)
    glGenFramebuffers(1, &pp->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);

    // Crear la textura de color
    glGenTextures(1, &pp->texture);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp->texture, 0);

    // Crear Renderbuffer para Depth/Stencil (Necesario para el 3D)
    glGenRenderbuffers(1, &pp->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, pp->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pp->rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("ERROR: Framebuffer no está completo!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void post_init(PostProcessSystem *pp, int w, int h)
{
    // 1. Inicializar IDs y configurar Buffers (FBO, Textura, RBO)
    pp->fbo = 0; pp->texture = 0; pp->rbo = 0;
    post_setup_buffers(pp, w, h);

    // 2. Definir el Quad (dos triángulos) que cubre toda la pantalla (-1 a 1 en NDC)
    // Cada vértice tiene: Posición (x, y) y Coordenadas de Textura (u, v)
    float vertices[] = {
        // Posicion    // UV
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // 3. Crear y configurar VAO y VBO para el Quad
    glGenVertexArrays(1, &pp->vao);
    glGenBuffers(1, &pp->vbo);
    
    glBindVertexArray(pp->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Atributo 0: Posición (x, y)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    
    // Atributo 1: Coordenadas UV (u, v)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    // 4. Cargar y compilar Shaders
    char *vert_src = load_shader_source("src/postprocesado/simple.vert");
    char *frag_src = load_shader_source("src/postprocesado/simple.frag");

    if (!vert_src || !frag_src) {
        printf("ERROR: No se pudieron cargar los archivos de shader de postprocesado\n");
        return;
    }

    // Vertex Shader
    GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh, 1, (const char**)&vert_src, NULL);
    glCompileShader(vsh);

    // Fragment Shader
    GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh, 1, (const char**)&frag_src, NULL);
    glCompileShader(fsh);

    // 5. Crear el Programa de Shader y enlazarlo
    pp->program = glCreateProgram();
    glAttachShader(pp->program, vsh);
    glAttachShader(pp->program, fsh);
    glLinkProgram(pp->program);

    // 6. OPTIMIZACIÓN: Cachear ubicaciones de Uniforms
    // Buscamos iTime e iResolution una sola vez aquí para no hacerlo en el bucle principal
    pp->timeLoc = glGetUniformLocation(pp->program, "iTime");
    pp->resLoc  = glGetUniformLocation(pp->program, "iResolution");

    // 7. Limpieza de recursos de compilación
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    SDL_free(vert_src);
    SDL_free(frag_src);
    
    printf("Post-procesado inicializado (Shader: simple.frag)\n");
    if (pp->timeLoc == -1) printf("Nota: El shader no usa 'iTime' o está optimizado.\n");
}

void post_begin(PostProcessSystem *pp)
{    
    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);
    glViewport(0, 0, pp->width, pp->height);
    glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void post_end(PostProcessSystem *pp)
{
    // Volvemos al buffer de la ventana principal
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B, CLEAR_COLOR_A);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dibujamos el quad con la textura procesada
    glUseProgram(pp->program);
    glBindVertexArray(pp->vao);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

#endif