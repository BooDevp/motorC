#ifndef GL_HEADERS_H
#define GL_HEADERS_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>

// Incluir OpenGL headers para Windows
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

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

// Funciones de shaders
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

// VARIABLES GLOBALES OPENGL (Extern)
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// Log function declaration
void debug_log(const char* format, ...);

// Loader function
bool load_opengl_functions(void);

#endif // GL_HEADERS_H
