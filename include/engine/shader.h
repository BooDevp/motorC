/**
 * Funciones para cargar y compilar shaders OpenGL
 * desde archivos externos.
 * Utiliza SDL_LoadFile para leer los archivos.
 */

#ifndef SHADER_H
#define SHADER_H

/**
 * Carga el código fuente de un shader desde un archivo
 */
static char *load_shader_source(const char *filename)
{
    size_t dataSize = 0;
    void *data = SDL_LoadFile(filename, &dataSize);

    if (data == NULL)
    {
        debug_log("ERROR cargando archivo shader %s: %s", filename, SDL_GetError());
        return NULL;
    }

    // SDL_LoadFile data is guaranteed to be null-terminated in SDL3
    return (char *)data;
}

/**
 * Compila un shader individual
 */
static GLuint compile_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Verificar errores
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        debug_log("ERROR compilando shader: %s", info_log);
        return 0;
    }

    return shader;
}

/**
 * Crea un programa de shaders
 */
static inline GLuint create_shader_program(void)
{
    char *vertex_source = load_shader_source("src/shaders/simple.vert");
    char *fragment_source = load_shader_source("src/shaders/simple.frag");

    if (!vertex_source || !fragment_source)
    {
        if (vertex_source)
            SDL_free(vertex_source);
        if (fragment_source)
            SDL_free(fragment_source);
        return 0;
    }

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

    SDL_free(vertex_source);
    SDL_free(fragment_source);

    if (vertex_shader == 0 || fragment_shader == 0)
    {
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

    if (!success)
    {
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


#endif // SHADER_H