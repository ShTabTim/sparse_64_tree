#ifndef HIGHGL_HPP
#define HIGHGL_HPP

#include <Windows.h>
#include <gl/GL.h>

typedef char GLchar;
typedef unsigned int GLenum;
typedef int GLint;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef ptrdiff_t GLsizeiptr;
//Buffers

#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88ec
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_SHADER_STORAGE_BUFFER 0x90D2

#define GL_STREAM_DRAW 0x88e0
#define GL_DYNAMIC_DRAW 0x88e8
#define GL_STATIC_DRAW 0x88e4

#define GL_UNPACK_ALIGNMENT 0xcf5

//Shaders

#define GL_COMPUTE_SHADER 0x91B9
#define GL_VERTEX_SHADER 0x8B31
#define GL_TESS_CONTROL_SHADER 0x8E88
#define GL_TESS_EVALUATION_SHADER 0x8E87
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_FRAGMENT_SHADER 0x8B30

#define GL_SHADER_TYPE 0x8B4F
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_SHADER_SOURCE_LENGTH 0x8B88

#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_ATTACHED_SHADERS 0x8B85

#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87

#define GL_RGBA32F 0x8814
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA

#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020

#define HIGH_GL_USING_LIST(X) \
\
X(void, glGenBuffers, GLsizei n, GLuint* buffers)\
X(void, glBindBuffer, GLenum target, GLuint buffer)\
X(void, glBufferData, GLenum target, GLsizeiptr size, const void* data, GLenum usage)\
X(void, glBindBufferBase, GLenum target, GLuint index, GLuint buffer)\
X(void, glDeleteBuffers, GLsizei n, const GLuint* buffers)\
\
X(GLuint, glCreateShader, GLenum shaderType)\
X(GLboolean, glIsShader, GLuint shader)\
X(void, glShaderSource, GLuint shader, GLsizei count, const GLchar** string, const GLint* length)\
X(void, glCompileShader, GLuint shader)\
X(void, glGetShaderiv, GLuint shader, GLenum pname, GLint* params)\
X(void, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog)\
X(void, glDeleteShader, GLuint shader)\
\
X(GLuint, glCreateProgram, void)\
X(GLboolean, glIsProgram, GLuint program)\
X(void, glGetAttachedShaders, GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)\
X(void, glAttachShader, GLuint program, GLuint shader)\
X(void, glDetachShader, GLuint program, GLuint shader)\
X(void, glLinkProgram, GLuint program)\
X(void, glGetProgramiv, GLuint program, GLenum pname, GLint* params)\
X(void, glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog)\
X(void, glDeleteProgram, GLuint program)\
X(void, glUseProgram, GLuint program)\
\
X(GLint, glGetUniformLocation, GLuint program, const GLchar* name)\
X(void, glGetActiveUniform, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)\
X(void, glUniform1ui, GLint location, GLuint v0)\
X(void, glUniform1f, GLint location, GLfloat v0)\
X(void, glUniform2f, GLint location, GLfloat v0, GLfloat v1)\
X(void, glUniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)\
X(void, glUniform4f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)\
\
X(void, glGenVertexArrays, GLsizei n, GLuint* arrays)\
X(void, glBindVertexArray, GLuint array)\
X(void, glDeleteVertexArrays, GLsizei n, GLuint* arrays)\
\
X(void, glBindImageTexture, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)\
X(void, glDispatchCompute, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)\
X(void, glMemoryBarrier, GLbitfield barriers)\
X(void, glMemoryBarrierByRegion, GLbitfield barriers)\
\
X(GLboolean, glIsBuffer, GLuint buffer)

#define high_gl_def(output, func, ...) typedef output(WINAPI* func##_proc)(__VA_ARGS__); extern func##_proc func;

HIGH_GL_USING_LIST(high_gl_def)

#undef high_gl_def

void high_gl_init();

#endif//HIGHGL_HPP