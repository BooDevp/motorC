#ifndef GL_HEADERS_H
#define GL_HEADERS_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

// ============================================================================
// CONFIGURACIÓN DE OPENGL POR PLATAFORMA
// ============================================================================
#ifdef _WIN32
    #include <windows.h>
    #include <GL/gl.h>
#else
    #include <SDL3/SDL_opengl.h>
#endif

// Tipos básicos de OpenGL 3.3+ (necesarios para Windows)
#ifndef GLuint
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#endif

// Constantes de OpenGL Moderno
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER    0x8892
#define GL_STATIC_DRAW     0x88E4
#define GL_VERTEX_SHADER   0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#endif

// ============================================================================
// DEFINICIÓN DE TIPOS DE FUNCIÓN (TYPEDEFS)
// ============================================================================
typedef GLuint (__stdcall *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (__stdcall *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
typedef void (__stdcall *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (__stdcall *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (__stdcall *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (__stdcall *PFNGLDELETESHADERPROC)(GLuint shader);

typedef GLuint (__stdcall *PFNGLCREATEPROGRAMPROC)(void);
typedef void (__stdcall *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (__stdcall *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (__stdcall *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (__stdcall *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (__stdcall *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void (__stdcall *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint (__stdcall *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void (__stdcall *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

typedef void (__stdcall *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (__stdcall *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (__stdcall *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (__stdcall *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (__stdcall *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (__stdcall *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (__stdcall *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (__stdcall *PFNGLDELETEARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (__stdcall *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);

// ============================================================================
// MACRO DE CONTROL PARA IMPLEMENTACIÓN UNICA
// ============================================================================
#ifdef GL_IMPLEMENTATION
    #define GL_DEF 
    #define GL_INIT(x) = x
#else
    #define GL_DEF extern
    #define GL_INIT(x)
#endif

// ============================================================================
// VARIABLES GLOBALES (PUNTEROS A FUNCIONES)
// ============================================================================
GL_DEF PFNGLCREATESHADERPROC glCreateShader GL_INIT(NULL);
GL_DEF PFNGLSHADERSOURCEPROC glShaderSource GL_INIT(NULL);
GL_DEF PFNGLCOMPILESHADERPROC glCompileShader GL_INIT(NULL);
GL_DEF PFNGLGETSHADERIVPROC glGetShaderiv GL_INIT(NULL);
GL_DEF PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog GL_INIT(NULL);
GL_DEF PFNGLDELETESHADERPROC glDeleteShader GL_INIT(NULL);

GL_DEF PFNGLCREATEPROGRAMPROC glCreateProgram GL_INIT(NULL);
GL_DEF PFNGLATTACHSHADERPROC glAttachShader GL_INIT(NULL);
GL_DEF PFNGLLINKPROGRAMPROC glLinkProgram GL_INIT(NULL);
GL_DEF PFNGLGETPROGRAMIVPROC glGetProgramiv GL_INIT(NULL);
GL_DEF PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog GL_INIT(NULL);
GL_DEF PFNGLDELETEPROGRAMPROC glDeleteProgram GL_INIT(NULL);
GL_DEF PFNGLUSEPROGRAMPROC glUseProgram GL_INIT(NULL);
GL_DEF PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation GL_INIT(NULL);
GL_DEF PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv GL_INIT(NULL);

GL_DEF PFNGLGENVERTEXARRAYSPROC glGenVertexArrays GL_INIT(NULL);
GL_DEF PFNGLBINDVERTEXARRAYPROC glBindVertexArray GL_INIT(NULL);
GL_DEF PFNGLGENBUFFERSPROC glGenBuffers GL_INIT(NULL);
GL_DEF PFNGLBINDBUFFERPROC glBindBuffer GL_INIT(NULL);
GL_DEF PFNGLBUFFERDATAPROC glBufferData GL_INIT(NULL);
GL_DEF PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer GL_INIT(NULL);
GL_DEF PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray GL_INIT(NULL);
GL_DEF PFNGLDELETEARRAYSPROC glDeleteVertexArrays GL_INIT(NULL);
GL_DEF PFNGLDELETEBUFFERSPROC glDeleteBuffers GL_INIT(NULL);

// Declaraciones de funciones
bool load_opengl_functions(void);
void debug_log(const char* format, ...);

// ============================================================================
// IMPLEMENTACIÓN (Solo si GL_IMPLEMENTATION está definido)
// ============================================================================
#ifdef GL_IMPLEMENTATION

void debug_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

bool load_opengl_functions(void) {
    debug_log("Cargando funciones de OpenGL (Header-Only)...");

    glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");

    glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");

    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glDeleteVertexArrays = (PFNGLDELETEARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");

    if (!glCreateShader || !glCreateProgram || !glGenVertexArrays || !glGenBuffers) {
        debug_log("ERROR: Fallo al cargar funciones críticas de OpenGL 3.3");
        return false;
    }

    debug_log("OpenGL cargado con éxito.");
    return true;
}

#endif // GL_IMPLEMENTATION

#endif // GL_HEADERS_H