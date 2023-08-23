#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace graphics
{
  class ShaderProgram
  {
  public:
    ShaderProgram(const char *vertex_shader_path, const char *fragment_shader_path);
    ~ShaderProgram();

  public:
    void use() const;

  public:
    void set_uniform(const char* name, float value);

    void set_uniform(const char* name, glm::vec2 value);
    void set_uniform(const char* name, glm::vec3 value);
    void set_uniform(const char* name, glm::vec4 value);

    void set_uniform(const char* name, glm::mat2 value);
    void set_uniform(const char* name, glm::mat3 value);
    void set_uniform(const char* name, glm::mat4 value);

  private:
    GLuint m_id;
  };
}

#endif // SHADER_PROGRAM_HPP
