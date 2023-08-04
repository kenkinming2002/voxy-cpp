#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <glad/glad.h>

class ShaderProgram
{
public:
  ShaderProgram(const char *vertex_shader_path, const char *fragment_shader_path);
  ~ShaderProgram();

public:
  GLuint id() const { return m_id; }

private:
  GLuint m_id;
};

#endif // SHADER_PROGRAM_HPP
