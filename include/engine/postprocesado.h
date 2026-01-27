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
    pp->fbo = 0; pp->texture = 0; pp->rbo = 0;
    post_setup_buffers(pp, w, h);

    // Quad que cubre toda la pantalla (-1 a 1 en NDC)
    float vertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &pp->vao);
    glGenBuffers(1, &pp->vbo);
    glBindVertexArray(pp->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    char *vert_src = load_shader_source("src/postprocesado/simple.vert");
    char *frag_src = load_shader_source("src/postprocesado/simple.frag");

    if (!vert_src || !frag_src) {
        printf("ERROR: No se pudieron cargar los archivos de shader de postprocesado\n");
        return;
    }

    GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh, 1, (const char**)&vert_src, NULL);
    glCompileShader(vsh);

    GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh, 1, (const char**)&frag_src, NULL);
    glCompileShader(fsh);

    pp->program = glCreateProgram();
    glAttachShader(pp->program, vsh);
    glAttachShader(pp->program, fsh);
    glLinkProgram(pp->program);

    // Limpieza de recursos temporales
    glDeleteShader(vsh);
    glDeleteShader(fsh);
    
    // Liberar la memoria asignada por SDL_LoadFile
    SDL_free(vert_src);
    SDL_free(frag_src);
}

void post_begin(PostProcessSystem *pp)
{    
    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);
    glViewport(0, 0, pp->width, pp->height);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void post_end(PostProcessSystem *pp)
{
    // Volvemos al buffer de la ventana principal
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dibujamos el quad con la textura procesada
    glUseProgram(pp->program);
    glBindVertexArray(pp->vao);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

#endif