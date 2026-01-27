#ifndef POSTPROCESADO_H
#define POSTPROCESADO_H

#include "external/gl_headers.h"
#include "shader.h"
#include <stdio.h>

#define POST_VERT_PATH "src/shaders/postprocesado/simple.vert"
#define POST_FRAG_PATH "src/shaders/postprocesado/retro.frag"

typedef struct
{
    GLuint fbo, texture, rbo, vao, vbo, program;
    int width, height;
    GLint timeLoc, resLoc, screenTexLoc;
} PostProcessSystem;

// Recrea los buffers al cambiar tamaño de ventana
static inline void post_setup_buffers(PostProcessSystem *pp, int w, int h)
{
    pp->width = w;
    pp->height = h;

    if (pp->fbo != 0)
        glDeleteFramebuffers(1, &pp->fbo);
    if (pp->texture != 0)
        glDeleteTextures(1, &pp->texture);
    if (pp->rbo != 0)
        glDeleteRenderbuffers(1, &pp->rbo);

    glGenFramebuffers(1, &pp->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);

    glGenTextures(1, &pp->texture);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pp->texture, 0);

    glGenRenderbuffers(1, &pp->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, pp->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pp->rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static inline void post_init(PostProcessSystem *pp, int w, int h)
{
    // Cargar punteros de funciones
    if (glActiveTexture == NULL)
        glActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
    if (glUniform1i == NULL)
        glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");

    pp->fbo = 0;
    pp->texture = 0;
    pp->rbo = 0;
    post_setup_buffers(pp, w, h);

    // Quad
    float vertices[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &pp->vao);
    glGenBuffers(1, &pp->vbo);
    glBindVertexArray(pp->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    // Compilación de Shaders
    char *vert_src = load_shader_source(POST_VERT_PATH);
    char *frag_src = load_shader_source(POST_FRAG_PATH);

    pp->program = glCreateProgram();
    GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh, 1, (const char **)&vert_src, NULL);
    glCompileShader(vsh);
    GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh, 1, (const char **)&frag_src, NULL);
    glCompileShader(fsh);

    glAttachShader(pp->program, vsh);
    glAttachShader(pp->program, fsh);
    glLinkProgram(pp->program);

    // --- CACHEO DE UNIFORMS ---
    pp->timeLoc = glGetUniformLocation(pp->program, "iTime");
    pp->resLoc = glGetUniformLocation(pp->program, "iResolution");
    pp->screenTexLoc = glGetUniformLocation(pp->program, "screenTexture");

    glDeleteShader(vsh);
    glDeleteShader(fsh);
    SDL_free(vert_src);
    SDL_free(frag_src);
    glBindVertexArray(0);
}

static inline void post_begin(PostProcessSystem *pp)
{
    glBindFramebuffer(GL_FRAMEBUFFER, pp->fbo);
    glViewport(0, 0, pp->width, pp->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

static inline void post_end(PostProcessSystem *pp)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(pp->program);

    // Configuración de texturas optimizada
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pp->texture);
    if (pp->screenTexLoc != -1)
        glUniform1i(pp->screenTexLoc, 0);

    glBindVertexArray(pp->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Reset para no afectar a otros renders
    glBindVertexArray(0);
    glUseProgram(0);
}

#endif