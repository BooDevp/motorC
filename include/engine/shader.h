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
/**
 * Crea un programa de shaders con rutas personalizadas
 */
static inline GLuint create_custom_shader_program(const char *vertex_path, const char *fragment_path)
{
    char *vertex_source = load_shader_source(vertex_path);
    char *fragment_source = load_shader_source(fragment_path);

    if (!vertex_source)
        debug_log("ERROR: Fallo al cargar VERTEX shader: %s", vertex_path);

    if (!fragment_source)
        debug_log("ERROR: Fallo al cargar FRAGMENT shader: %s", fragment_path);

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

    debug_log("Programa de shaders creado (ID: %u) [%s, %s]", program, vertex_path, fragment_path);
    return program;
}

/**
 * Crea el programa de shaders por defecto
 */
static inline GLuint create_shader_program(void)
{
    return create_custom_shader_program("src/shaders/simple.vert", "src/shaders/simple.frag");
}

/**
 * Funciones auxiliares para establecer uniforms en shaders
 */

// Establecer uniform float
static inline void shader_set_float(GLuint program, const char *name, float value)
{
    GLint loc = glGetUniformLocation(program, name);
    if (loc != -1)
    {
        glUniform1f(loc, value);
    }
}

// Establecer uniform vec2
static inline void shader_set_vec2(GLuint program, const char *name, float x, float y)
{
    GLint loc = glGetUniformLocation(program, name);
    if (loc != -1)
    {
        glUniform2f(loc, x, y);
    }
}

// Establecer uniform vec3
static inline void shader_set_vec3(GLuint program, const char *name, float x, float y, float z)
{
    GLint loc = glGetUniformLocation(program, name);
    if (loc != -1)
    {
        glUniform3f(loc, x, y, z);
    }
}

// Establecer uniform vec4
static inline void shader_set_vec4(GLuint program, const char *name, float x, float y, float z, float w)
{
    GLint loc = glGetUniformLocation(program, name);
    if (loc != -1)
    {
        glUniform4f(loc, x, y, z, w);
    }
}

// Establecer uniform int
static inline void shader_set_int(GLuint program, const char *name, int value)
{
    GLint loc = glGetUniformLocation(program, name);
    if (loc != -1)
    {
        glUniform1i(loc, value);
    }
}

#endif // SHADER_H