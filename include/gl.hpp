#ifndef GL_HPP
#define GL_HPP

#include <glad/glad.h>

#include <utility>

namespace gl
{
#define GL_HPP_TYPE(type, args)                                                       \
  struct type                                                                         \
  {                                                                                   \
    type(args);                                                                       \
    ~type();                                                                          \
    type(const type& other) = delete;                                                 \
    type& operator=(const type& other) = delete;                                      \
    type(type&& other) : handle(0) { std::swap(handle, other.handle); };              \
    type& operator=(type&& other) { std::swap(handle, other.handle); return *this; }; \
    operator GLuint() { return handle; }                                              \
    GLuint handle;                                                                    \
  }

  GL_HPP_TYPE(VertexArray,);
  GL_HPP_TYPE(Buffer,);
  GL_HPP_TYPE(Shader, GLenum type);
  GL_HPP_TYPE(Program,);
  GL_HPP_TYPE(Texture,);

  gl::Shader compile_shader(GLenum type, const char* path);
  gl::Program compile_program(const char* vertex_shader_path, const char *fragment_shader_path);
  gl::Texture load_texture(const char *path);

  void init_debug();
}

#endif // GL_HPP
